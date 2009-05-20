#include <pspsdk.h>
#include <pspkernel.h>
#include <string.h>
#include <stdio.h>

PSP_MODULE_INFO("klauncher", 0x1000, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER);

#define ARG_MAX 19
char *argv[ARG_MAX + 1];

int klauncher_thread (SceSize args, char* argp)
{
  SceUID modid = sceKernelLoadModule(argp, 0, NULL);
  if (modid > 0)
    {
      int mresult;
      sceKernelStartModule(modid, args, argp, &mresult, NULL);
    }
  
  // Completely free all memory
  sceKernelSelfStopUnloadModule(1, 0, NULL);

  return 0;
}

int
module_start (SceSize args, char* argp)
{
  char buf[100];
  SceUID fd = sceIoOpen ("host0:/klauncher.txt",
			 PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);
  if (fd < 0)
    return -1;

  sceIoWrite(fd, argp, args);
  sceIoWrite(fd, "\nend\n", 5);

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
  sceIoClose(fd);
  return 0;
}


/*   /\* From pspsdk/src/startup/crt0_prx.c *\/ */
/*   int argc = 0; */
/*   int loc = 0; */
/*   char *ptr = argp; */
  
/*   /\* Turn our thread arguments into main()'s argc and argv[]. *\/ */
/*   while(loc < args) */
/*     { */
/*       argv[argc] = &ptr[loc]; */
/*       loc += strlen(&ptr[loc]) + 1; */
/*       argc++; */
/*       if (argc == ARG_MAX) */
/* 	  break; */
/*     } */
/*   argv[argc] = NULL; */

/*   argv[0] = "host0:/freedink/cross-psp/src/freedink.prx"; */


/*   sprintf(buf, "argc = %d\n", argc); */
/*   sceIoWrite(fd, buf, strlen(buf)); */
/*   char** arg; */
/*   for (arg = argv; *arg != NULL; arg++) */
/*     { */
/*       /\* printf("%s\n", *arg); *\/ */
/*       sceIoWrite(fd, *arg, strlen(*arg)); */
/*       sceIoWrite(fd, "\n", 1); */
/*     } */
/*   sceIoClose(fd); */
