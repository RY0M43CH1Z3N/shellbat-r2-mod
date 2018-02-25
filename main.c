
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/power.h>
#include <taihen.h>

static SceUID g_hooks[2];



#if 0
static tai_hook_ref_t ref_hook;
// offset 0x1844f0
static int status_draw_battery_patched(int a1, uint8_t a2)
{
    return TAI_CONTINUE(int, ref_hook, a1, a2);
}
#endif

static int in_draw_time = 0;

static int ampm_start = -1;
static int bat_num_start = 0;
static int bat_num_len = 0;
static int percent_start = 0;

static int digit_len(int num)
{
    if (num < 10) {
        return 1;
    } else if (num < 100) {
        return 2;
    } else {
        return 3;
    }
}


static tai_hook_ref_t ref_hook0;
static int status_draw_time_patched(int a1, int a2)
{
    in_draw_time = 1;
    int out = TAI_CONTINUE(int, ref_hook0, a1, a2);
    in_draw_time = 0;
    return out;
}

static tai_hook_ref_t ref_hook1;
static uint16_t **some_strdup_patched(uint16_t **a1, uint16_t *a2, int a2_size)
{
    if (in_draw_time) {
        static int oldpercent = 0;
        int percent = scePowerGetBatteryLifePercent();
        if (percent < 0 || percent > 100) {
            percent = oldpercent;
        }
        oldpercent = percent;
        char buff[10];
        int len = sceClibSnprintf(buff, 10, "  %d%% ", percent);
        for (int i = 0; i < len; ++i) {
            a2[a2_size + i] = buff[i];
        }
        a2[a2_size + len] = 0;


        if (a2[a2_size - 1] == 'M') {
            ampm_start = a2_size - 2;
        } else {
            ampm_start = -1;
        }
        bat_num_start = a2_size + 2;
        bat_num_len = digit_len(percent);
        percent_start = bat_num_start + bat_num_len;



        return TAI_CONTINUE(uint16_t**, ref_hook1, a1, a2, a2_size + len);
    }
    return TAI_CONTINUE(uint16_t**, ref_hook1, a1, a2, a2_size);
}

void _start() __attribute__ ((weak, alias ("module_start")));
int module_start(SceSize argc, const void *args){

    uint32_t offsets[2];

    offsets[0] = -1;

    offsets[1] = -1;



    tai_module_info_t info;
    info.size = sizeof(info);
    if (taiGetModuleInfo("SceShell", &info) >= 0) {



        switch (info.module_nid) {
        case 0x0552F692: { // retail 3.60 SceShell

		offsets[0] = 0x183EA4;

		offsets[1] = 0x40E0B4;

		break;
        }


	case 0x6CB01295: { // PDEL 3.60 SceShell

		offsets[0] = 0x17B8DC;

		offsets[1] = 0x400028;

		break;

	}


	case 0xEAB89D5C: { // PTEL 3.60 SceShell

		offsets[0] = 0x17C2D8;

		offsets[1] = 0x404828;

		break;

	}


        default:
            //LOG("SceShell %XNID not recognized", info.module_nid);
            break;
        }


            if(offsets[0] >= 0)g_hooks[0] = taiHookFunctionOffset(&ref_hook0,
                                               info.modid,
                                               0,         // segidx
                                               offsets[0],  // offset
                                               1,         // thumb
                                               status_draw_time_patched);

            if(offsets[1] >= 0)g_hooks[1] = taiHookFunctionOffset(&ref_hook1,
                                               info.modid,
                                               0,         // segidx
                                               offsets[1],  // offset
                                               1,         // thumb
                                               some_strdup_patched);




    }

    return SCE_KERNEL_START_SUCCESS;
}

int module_stop(SceSize argc, const void *args){

    if (g_hooks[0] >= 0) taiHookRelease(g_hooks[0], ref_hook0);
    if (g_hooks[1] >= 0) taiHookRelease(g_hooks[1], ref_hook1);

    return SCE_KERNEL_STOP_SUCCESS;
}
