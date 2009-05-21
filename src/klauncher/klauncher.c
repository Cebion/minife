#include <pspkernel.h>

PSP_MODULE_INFO("klauncher", PSP_MODULE_KERNEL, 1, 0);
PSP_MAIN_THREAD_ATTR(0); // kernel thread

int klauncher_thread (SceSize args, char* argp)
{
  SceUID modid = sceKernelLoadModule(argp, 0, NULL);
  if (modid > 0)
    {
      int ignored_mresult;
      sceKernelStartModule(modid, args, argp, &ignored_mresult, NULL);
    }
  
  // Completely free all memory
  sceKernelSelfStopUnloadModule(1, 0, NULL);

  return 0;
}

int
module_start (SceSize args, char* argp)
{
  /* We need to run sceKernelLoadModule from a thread */
  SceUID thread = sceKernelCreateThread("klauncher_thread",
					(void*)klauncher_thread, 0x30, 0x10000, 0, NULL);
  if (thread >= 0) {
    /* Drop first argument (path to current module) */
    int i = 0;
    while(argp[i] != '\0') i++;
    i++;
    sceKernelStartThread (thread, args-i, argp+i);
  }
  return 0;
}
