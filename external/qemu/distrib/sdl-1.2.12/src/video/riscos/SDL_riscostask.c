
#include "SDL_config.h"


#include "kernel.h"
#include "swis.h"

#include "SDL_stdinc.h"
#include "SDL_riscostask.h"

#if !SDL_THREADS_DISABLED
#include <pthread.h>
pthread_t main_thread;
#endif

/* RISC OS variables */

static int task_handle = 0;
static int wimp_version = 0;

/* RISC OS variables to help compatability with certain programs */
int riscos_backbuffer = 0; /* Create a back buffer in system memory for full screen mode */
int riscos_closeaction = 1; /* Close icon action */

static int stored_mode = -1; /* -1 when in desktop, mode number or pointer when full screen */

extern int mouseInWindow; /* Mouse is in WIMP window */

/* Local function */

static int RISCOS_GetTaskName(char *task_name, size_t maxlen);

/* Uncomment next line to copy mode changes/restores to stderr */
/* #define DUMP_MODE */
#ifdef DUMP_MODE
#include "stdio.h"
static void dump_mode()
{
    fprintf(stderr, "mode %d\n", stored_mode);
    if (stored_mode < -1 || stored_mode >= 256)
    {
        int blockSize = 0;
		int *storeBlock = (int *)stored_mode;

        while(blockSize < 5 || storeBlock[blockSize] != -1)
        {
           fprintf(stderr, "   %d\n", storeBlock[blockSize++]);
        }
    }
}
#endif


int RISCOS_InitTask()
{
   char task_name[32];
   _kernel_swi_regs regs;
   int messages[4];

   if (RISCOS_GetTaskName(task_name, SDL_arraysize(task_name)) == 0) return 0;

   messages[0] = 9;       /* Palette changed */
   messages[1] = 0x400c1; /* Mode changed */
   messages[2] = 8;       /* Pre quit */
   messages[2] = 0;
   
	regs.r[0] = (unsigned int)360; /* Minimum version 3.6 */
	regs.r[1] = (unsigned int)0x4b534154;
	regs.r[2] = (unsigned int)task_name;
	regs.r[3] = (unsigned int)messages;

   if (_kernel_swi(Wimp_Initialise, &regs, &regs) == 0)
   {
	   wimp_version = regs.r[0];
	   task_handle = regs.r[1];
	   return 1;
   }

#if !SDL_THREADS_DISABLED
   main_thread = pthread_self();
#endif

   return 0;
}


void RISCOS_ExitTask()
{
	_kernel_swi_regs regs;

    if (stored_mode == -1)
    {
       /* Ensure cursor is put back to standard pointer shape if
          we have been running in a window */
       _kernel_osbyte(106,1,0);
    }

	/* Ensure we end up back in the wimp */
	RISCOS_RestoreWimpMode();

	/* Neatly exit the task */
   	regs.r[0] = task_handle;
   	regs.r[1] = (unsigned int)0x4b534154;
   	_kernel_swi(Wimp_CloseDown, &regs, &regs);
	task_handle = 0;
}


int RISCOS_GetTaskName(char *task_name, size_t maxlen)
{
	_kernel_swi_regs regs;

   task_name[0] = 0;

   /* Figure out a sensible task name */
   if (_kernel_swi(OS_GetEnv, &regs, &regs) == 0)
   {
	   char *command_line = (char *)regs.r[0];
	   size_t len = SDL_strlen(command_line)+1;
	   char *buffer = SDL_stack_alloc(char, len);
	   char *env_var;
	   char *p;

	   SDL_strlcpy(buffer, command_line, len);
	   p = SDL_strchr(buffer, ' ');
	   if (p) *p = 0;
	   p = SDL_strrchr(buffer, '.');
	   if (p == 0) p = buffer;
	   if (stricmp(p+1,"!RunImage") == 0)
	   {
		   *p = 0;
	   	   p = SDL_strrchr(buffer, '.');
		   if (p == 0) p = buffer;
	   }
	   if (*p == '.') p++;
	   if (*p == '!') p++; /* Skip "!" at beginning of application directories */

       if (*p == '<')
       {
          // Probably in the form <appname$Dir>
          char *q = SDL_strchr(p, '$');
          if (q == 0) q = SDL_strchr(p,'>'); /* Use variable name if not */
          if (q) *q = 0;
          p++; /* Move over the < */
       }

	   if (*p)
	   {
		   /* Read variables that effect the RISC OS SDL engine for this task */
		   len = SDL_strlen(p) + 18; /* 18 is larger than the biggest variable name */
		   env_var = SDL_stack_alloc(char, len);
		   if (env_var)
		   {
			   char *env_val;

			   /* See if a variable of form SDL$<dirname>$TaskName exists */

			   SDL_strlcpy(env_var, "SDL$", len);
			   SDL_strlcat(env_var, p, len);
			   SDL_strlcat(env_var, "$TaskName", len);

			   env_val = SDL_getenv(env_var);
			   if (env_val) SDL_strlcpy(task_name, env_val, maxlen);

			   SDL_strlcpy(env_var, "SDL$", len);
			   SDL_strlcat(env_var, p, len);
			   SDL_strlcat(env_var, "$BackBuffer", len);

			   env_val = SDL_getenv(env_var);
			   if (env_val) riscos_backbuffer = atoi(env_val);

			   SDL_strlcpy(env_var, "SDL$", len);
			   SDL_strlcat(env_var, p, len);
			   SDL_strlcat(env_var, "$CloseAction", len);

			   env_val = SDL_getenv(env_var);
			   if (env_val && SDL_strcmp(env_val,"0") == 0) riscos_closeaction = 0;

			   SDL_stack_free(env_var);
		   }
		   
		   if (!*task_name) SDL_strlcpy(task_name, p, maxlen);
	   }

	   SDL_stack_free(buffer);
   }

   if (task_name[0] == 0) SDL_strlcpy(task_name, "SDL Task", maxlen);

   return 1;
}


void RISCOS_StoreWimpMode()
{
     _kernel_swi_regs regs;

	/* Don't store if in full screen mode */
	if (stored_mode != -1) return;

    regs.r[0] = 1;
    _kernel_swi(OS_ScreenMode, &regs, &regs);
    if (regs.r[1] >= 0 && regs.r[1] < 256) stored_mode = regs.r[1];
    else
    {
        int blockSize = 0;
        int *retBlock = (int *)regs.r[1];
		int *storeBlock;
        int j;

        while(blockSize < 5 || retBlock[blockSize] != -1) blockSize++;
        blockSize++;
        storeBlock = (int *)SDL_malloc(blockSize * sizeof(int));
        retBlock = (int *)regs.r[1];
        for ( j = 0; j < blockSize; j++)
           storeBlock[j] = retBlock[j];

		stored_mode = (int)storeBlock;
     }
#if DUMP_MODE
    fprintf(stderr, "Stored "); dump_mode();
#endif
}


void RISCOS_RestoreWimpMode()
{
    _kernel_swi_regs regs;

	/* Only need to restore if we are in full screen mode */
	if (stored_mode == -1) return;

#if DUMP_MODE
   fprintf(stderr, "Restored"); dump_mode();
#endif

    regs.r[0] = stored_mode;
    _kernel_swi(Wimp_SetMode, &regs, &regs);
    if (stored_mode < 0 || stored_mode > 256)
    {
       SDL_free((int *)stored_mode);
    }
    stored_mode = -1;

    /* Flush keyboard buffer to dump the keystrokes we've already polled */
    regs.r[0] = 21;
    regs.r[1] = 0; /* Keyboard buffer number */
    _kernel_swi(OS_Byte, &regs, &regs);

    mouseInWindow = 0;

}


int RISCOS_GetWimpVersion()
{
	return wimp_version;
}

int RISCOS_GetTaskHandle()
{
	return task_handle;
}
