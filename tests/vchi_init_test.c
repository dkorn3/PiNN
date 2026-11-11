#include <stdio.h>
#include <stdlib.h>

#include "interface/vcos/vcos.h"
#include "interface/vchi/vchi.h"
#include "interface/vmcs_host/vc_vchi_gpuserv.h"

int main(void)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("[1] Initializing VCOS...\n");
    if (vcos_init() != VCOS_SUCCESS) {
        fprintf(stderr, "FAIL: vcos_init()\n");
        return 1;
    }

    VCHI_INSTANCE_T instance = NULL;

    printf("[2] Initializing VCHI...\n");
    if (vchi_initialise(&instance) != 0) {
        fprintf(stderr, "FAIL: vchi_initialise()\n");
        vcos_deinit();
        return 1;
    }

    printf("[3] Connecting VCHI...\n");
    if (vchi_connect(NULL, 0, instance) != 0) {
        fprintf(stderr, "FAIL: vchi_connect()\n");
        vchi_disconnect(instance);
        vcos_deinit();
        return 1;
    }

    printf("[4] Opening VideoCore GPUS service...\n");
    if (vc_gpuserv_init() != 0) {
        fprintf(stderr, "FAIL: vc_gpuserv_init()\n");
        vchi_disconnect(instance);
        vcos_deinit();
        return 1;
    }

    printf("SUCCESS: VCHI connected and GPUS service initialized.\n");

    printf("[5] Closing GPUS service...\n");
    vc_gpuserv_deinit();

    printf("[6] Disconnecting VCHI...\n");
    vchi_disconnect(instance);

    vcos_deinit();

    printf("Initialization test completed.\n");
    return 0;
}
