#include "../../stdafx.h"
#include "ajm_mp3_decoder.h"
#include "../logger/logger.h"

#pragma comment(lib, "libSceAjm_stub_weak.a")

namespace ajm
{
    void mp3_decoder::create()
    {
        int ret;

		ret = sceAjmInitialize(0, &m_context);
        if (ret != 0)
        {
            LOG_ERROR("sceAjmInitialize failed: 0x%x\n", ret);
        }

		ret = sceAjmModuleRegister(m_context, SCE_AJM_CODEC_MP3_DEC, 0);
		if (ret != 0)
		{
			LOG_ERROR("sceAjmModuleRegister(MP3_DEC) failed: 0x%x\n", ret);
			sceAjmFinalize(m_context);
			m_context = SCE_AJM_CONTEXT_INVALID;
		}
    }
    bool mp3_decoder::initialize(uint32_t max_channels, uint32_t source_rate, uint32_t target_rate)
    {
        int ret;

        const SceAjmFormatChannel ch_flag = (max_channels >= 2)
            ? SCE_AJM_FORMAT_CHANNEL_2CH
            : SCE_AJM_FORMAT_CHANNEL_1CH;

        const bool do_resample = (source_rate > 0 && target_rate > 0 && source_rate != target_rate);

        uint64_t instance_flags =
            SCE_AJM_INSTANCE_FLAG_MAX_CHANNEL(ch_flag) |
            SCE_AJM_INSTANCE_FLAG_FORMAT(SCE_AJM_FORMAT_ENCODING_S16);

        if (do_resample)
            instance_flags |= SCE_AJM_INSTANCE_FLAG_RESAMPLE;

        ret = sceAjmInstanceCreate(m_context, SCE_AJM_CODEC_MP3_DEC, instance_flags, &m_instance);
        if (ret != 0)
        {
            LOG_ERROR("sceAjmInstanceCreate(MP3_DEC) failed: 0x%x\n", ret);
            sceAjmModuleUnregister(m_context, SCE_AJM_CODEC_MP3_DEC);
            sceAjmFinalize(m_context);
            m_context  = SCE_AJM_CONTEXT_INVALID;
            return false;
        }

        if (do_resample)
        {
            const float ratio = static_cast<float>(source_rate) / static_cast<float>(target_rate);

            alignas(8) uint8_t batch_buf[SCE_AJM_JOB_SET_RESAMPLE_PARAMETERS_SIZE];
            SceAjmBatchInfo batch;
            sceAjmBatchInitialize(batch_buf, sizeof(batch_buf), &batch);

            SceAjmSidebandResult resample_result = {};
            ret = sceAjmBatchJobSetResampleParameters(&batch, m_instance,
                                                      ratio,
                                                      SCE_AJM_RESAMPLE_FLAG_RESET,
                                                      &resample_result);
            if (ret != 0)
            {
                LOG_ERROR("sceAjmBatchJobSetResampleParameters failed: 0x%x\n", ret);
                goto fail;
            }

            SceAjmBatchId batch_id;
            ret = sceAjmBatchStart(m_context, &batch, SCE_AJM_PRIORITY_GAME_DEFAULT, nullptr, &batch_id);
            if (ret != 0)
            {
                LOG_ERROR("sceAjmBatchStart(resample init) failed: 0x%x\n", ret);
                goto fail;
            }

            ret = sceAjmBatchWait(m_context, batch_id, SCE_AJM_WAIT_INFINITE, nullptr);
            if (ret != 0)
            {
                LOG_ERROR("sceAjmBatchWait(resample init) failed: 0x%x\n", ret);
                goto fail;
            }

            if (resample_result.iResult < 0)
            {
                LOG_ERROR("SetResampleParameters result: 0x%x\n", resample_result.iResult);
                goto fail;
            }

            LOG_INFO("AJM MP3 decoder initialized (resample %u -> %u Hz, ratio=%.4f)\n",
                     source_rate, target_rate, ratio);
        }
        else
        {
            LOG_INFO("AJM MP3 decoder initialized (no resampling, %u Hz)\n", source_rate);
        }

        m_initialized = true;
        return true;

    fail:
        sceAjmInstanceDestroy(m_context, m_instance);
        sceAjmModuleUnregister(m_context, SCE_AJM_CODEC_MP3_DEC);
        sceAjmFinalize(m_context);
        m_context  = SCE_AJM_CONTEXT_INVALID;
        m_instance = SCE_AJM_INSTANCE_INVALID;
        return false;
    }

    void mp3_decoder::finalize()
    {
        if (!m_initialized)
            return;

        sceAjmInstanceDestroy(m_context, m_instance);
        sceAjmModuleUnregister(m_context, SCE_AJM_CODEC_MP3_DEC);
        sceAjmFinalize(m_context);
        m_context     = SCE_AJM_CONTEXT_INVALID;
        m_instance    = SCE_AJM_INSTANCE_INVALID;
        m_initialized = false;

        LOG_INFO("AJM MP3 decoder finalized\n");
    }

    uint32_t mp3_decoder::decode(const void* mp3_data, size_t mp3_size,
                                 int16_t*    pcm_out,  size_t pcm_out_size,
                                 size_t*     bytes_consumed_out)
    {
        if (!m_initialized)
            return 0;

        alignas(8) uint8_t batch_buf[SCE_AJM_JOB_DECODE_SIZE];
        SceAjmBatchInfo batch;
        sceAjmBatchInitialize(batch_buf, sizeof(batch_buf), &batch);

        SceAjmDecodeResult dec_result = {};
        int ret = sceAjmBatchJobDecode(&batch, m_instance,
                                       mp3_data, mp3_size,
                                       pcm_out,  pcm_out_size,
                                       &dec_result);
        if (ret != 0)
        {
            LOG_ERROR("sceAjmBatchJobDecode failed: 0x%x\n", ret);
            return 0;
        }

        SceAjmBatchId batch_id;
        ret = sceAjmBatchStart(m_context, &batch, SCE_AJM_PRIORITY_GAME_DEFAULT, nullptr, &batch_id);
        if (ret != 0)
        {
            LOG_ERROR("sceAjmBatchStart(decode) failed: 0x%x\n", ret);
            return 0;
        }

        ret = sceAjmBatchWait(m_context, batch_id, SCE_AJM_WAIT_INFINITE, nullptr);
        if (ret != 0)
        {
            LOG_ERROR("sceAjmBatchWait(decode) failed: 0x%x\n", ret);
            return 0;
        }

        if (dec_result.sResult.iResult < 0)
        {
            LOG_ERROR("MP3 decode fatal error: 0x%x\n", dec_result.sResult.iResult);
            return 0;
        }

        if (bytes_consumed_out)
            *bytes_consumed_out = static_cast<size_t>(dec_result.sStream.iSizeConsumed);

        return static_cast<uint32_t>(dec_result.sStream.iSizeProduced);
    }
}
