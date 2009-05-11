#include <pspkernel.h>
#include <pspdebug.h>

#include <pspsdk.h>

#include <pspsysmem.h>
int _EXFUN(__psp_free_heap, (void));

#include <stdio.h>
#include <stdlib.h>

PSP_MODULE_INFO("Hello World", 0, 1, 1);

int exit_callback(int arg1, int arg2, void *common)
{
  printf("Bye bye!\n");
  sceKernelExitGame();
  return 0;
}
 
 
int CallbackThread(SceSize args, void *argp)
{
  int cbid;
 
  cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
  sceKernelRegisterExitCallback(cbid);
 
  sceKernelSleepThreadCB();
 
  return 0;
}
 
 
int SetupCallbacks(void)
{
  int thid = 0;
 
  thid = sceKernelCreateThread("update_thread", CallbackThread, 0x11, 0x10000, 0, NULL);
  if(thid >= 0)
    {
      sceKernelStartThread(thid, 0, 0);
    }
 
  return thid;
}

#define DEBUG(...) pspDebugScreenPrintf(__VA_ARGS__); printf(__VA_ARGS__);
int main()
{
  int initial_free_memory = sceKernelTotalFreeMemSize()/1024;
  int initial_max_free_block = sceKernelMaxFreeMemSize()/1024;

  pspDebugScreenInit();

  DEBUG("-- launch --\n");
  DEBUG("1:\n");
  DEBUG("Memory free: %dkB\n", initial_free_memory);
  DEBUG("Max free block: %dkB\n", initial_max_free_block);

  DEBUG("2:\n");
  int tmalloc = 0;
  while (malloc(1024) != NULL)
    tmalloc += 1024;
/*   while (malloc(1024*1024) != NULL) */
/*     tmalloc += 1024*1024; */
  printf("tmalloc = %d\n", tmalloc);
  /* FW 1.50 => 10910720 / 10485760 => < 24000kB */
  /* FW 5.00 => 39819264 / 39845888 => < 52500kB */
  /*               53047296 */
  DEBUG("Total allocable memory: %d\n", tmalloc);
  DEBUG("Memory free: %dkB\n", sceKernelTotalFreeMemSize()/1024);
  DEBUG("Max free block: %dkB\n", sceKernelMaxFreeMemSize()/1024);

  __psp_free_heap();
  DEBUG("3:\n");
  DEBUG("Memory free: %dkB\n", sceKernelTotalFreeMemSize()/1024);
  DEBUG("Max free block: %dkB\n", sceKernelMaxFreeMemSize()/1024);
  DEBUG("\n");

  /* Run Dink */
  char* myargv[] = { (char*)"-d" };
//   SceUID mod = pspSdkLoadStartModuleWithArgs("host0:/freedink/cross-psp/src/freedink.prx",
// 					     PSP_MEMORY_PARTITION_USER, 1, myargv);
  SceUID mod = pspSdkLoadStartModuleWithArgs("host0:/tutorials/meminfo-prx-large/hello.prx",
					     PSP_MEMORY_PARTITION_USER, 1, myargv);  if (mod < 0)
    {
      // Error
      pspDebugScreenInit();
      pspDebugScreenPrintf("Could not run FreeDink:\n");
      switch(mod)
	{
	case 0x80010002:
	  pspDebugScreenPrintf("Program not found (%p)\n", mod);
	  break;
	case 0x80020148: // SCE_KERNEL_ERROR_UNSUPPORTED_PRX_TYPE
	  pspDebugScreenPrintf("Unsupported PRX application (%p)\n", mod);
	  break;
	case 0x800200D9: // http://forums.ps2dev.org/viewtopic.php?t=11887
	  pspDebugScreenPrintf("Not enough memory (%p)\n", mod);
	  break;
	case 0x80020149: // SCE_KERNEL_ERROR_ILLEGAL_PERM_CALL
	  pspDebugScreenPrintf("Not running from memory card? (%p)\n", mod);
	  break;
	case 0x80010014:
	  pspDebugScreenPrintf("Invalid path? (%p)\n", mod);
	  break;
	default:
	  pspDebugScreenPrintf("Unknown error %p\n", mod);
	}
      pspDebugScreenPrintf("\n");
      //pspDebugScreenPrintf("If you think that's a bug, please write to " PACKAGE_BUGREPORT);
      sleep(5);
      /* Pause the thread, so that the user still can use the [HOME]
	 button to reset to the XMB. */
      sceKernelSleepThread();

      // never reached
      sceKernelExitGame();
      return EXIT_FAILURE;
    }
  // Don't printf anything here, it messes the SDL video
  // initialization for some reason..

  // Completely free all memory
  sceKernelSelfStopUnloadModule(1, 0, NULL);
  return EXIT_SUCCESS; // never reached
}
