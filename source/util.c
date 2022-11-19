#define _CRT_SECURE_NO_WARNINGS
#include "solnp_glbopts.h"
#include "solnp_util.h"

#if (defined NOTIMER)

void SOLNP(tic)
(
      SOLNP(timer) *t
) {}

solnp_float SOLNP(tocq)
(
      SOLNP(timer) *t
) 
{
      return NAN;
}

#elif(defined _WIN32 || _WIN64 || defined _WINDLL)

void SOLNP(tic)
(
      SOLNP(timer) *t
) 
{
      QueryPerformanceFrequency(&t->freq);
      QueryPerformanceCounter(&t->tic);
}

solnp_float SOLNP(tocq)
(
      SOLNP(timer) *t
) 
{
      QueryPerformanceCounter(&t->toc);
      return (1e3 * (t->toc.QuadPart - t->tic.QuadPart) / (solnp_float)t->freq.QuadPart);
}

#elif(defined __APPLE__)

void SOLNP(tic)
(
      SOLNP(timer) *t
) 
{
      /* read current clock cycles */
      t->tic = mach_absolute_time();
}

solnp_float SOLNP(tocq)
(
      SOLNP(timer) *t
) 
{
      uint64_t duration;

      t->toc = mach_absolute_time();
      duration = t->toc - t->tic;

      mach_timebase_info(&(t->tinfo));
      duration *= t->tinfo.numer;
      duration /= t->tinfo.denom;

      return (solnp_float)duration / 1e6;
}

#else

void SOLNP(tic)
(
      SOLNP(timer) *t
) 
{
      clock_gettime(CLOCK_MONOTONIC, &t->tic);
}

solnp_float SOLNP(tocq)
(
      SOLNP(timer) *t
) 
{
      struct timespec temp;

      clock_gettime(CLOCK_MONOTONIC, &t->toc);

      if ((t->toc.tv_nsec - t->tic.tv_nsec) < 0) 
      {
            temp.tv_sec = t->toc.tv_sec - t->tic.tv_sec - 1;
            temp.tv_nsec = 1e9 + t->toc.tv_nsec - t->tic.tv_nsec;
      } 
      else 
      {
            temp.tv_sec = t->toc.tv_sec - t->tic.tv_sec;
            temp.tv_nsec = t->toc.tv_nsec - t->tic.tv_nsec;
      }
      
      return (solnp_float) temp.tv_sec * 1e3 + (solnp_float) temp.tv_nsec / 1e6;
}

#endif

solnp_float SOLNP(toc)
(
      SOLNP(timer) *t
) 
{
      solnp_float time = SOLNP(tocq)(t);
      solnp_printf("time: %8.4f milli-seconds.\n", time);
      return time;
}

solnp_float SOLNP(str_toc)
(
      char *str, 
      SOLNP(timer) *t
) 
{
      solnp_float time = SOLNP(tocq)(t);
      solnp_printf("%s - time: %8.4f milli-seconds.\n", str, time);
      return time;
}


SOLNPConstraint *malloc_constriant(solnp_int n, solnp_int nic)
{
      SOLNPConstraint *constraint = (SOLNPConstraint *)solnp_calloc(1, sizeof(SOLNPConstraint));

      constraint->n = n;
      constraint->nic = nic;

      if(nic > 0){
            constraint->il = (solnp_float*)solnp_malloc(nic * sizeof(solnp_float));
            constraint->iu = (solnp_float*)solnp_malloc(nic * sizeof(solnp_float));
      }
      else{
            constraint->il = SOLNP_NULL;
            constraint->iu = SOLNP_NULL;
      }

      if(n > 0){
            constraint->pl = (solnp_float*)solnp_malloc(n * sizeof(solnp_float));
            constraint->pu = (solnp_float*)solnp_malloc(n * sizeof(solnp_float));
      }
      else{
            constraint->pl = SOLNP_NULL;
            constraint->pu = SOLNP_NULL;
      }

      constraint->Ipc = (solnp_float*)solnp_malloc(2 * sizeof(solnp_float));
      constraint->Ipb = (solnp_float*)solnp_malloc(2 * sizeof(solnp_float));

      return constraint;

}

SOLNPCost *malloc_cost(solnp_int nec, solnp_int nic, void *func)
{
      SOLNPCost *c = (SOLNPCost*)solnp_calloc(1, sizeof(SOLNPCost));

      c->nec = nec;
      c->nic = nic;

      if(nec > 0){
            c->ec = (solnp_float*)solnp_malloc(nec*sizeof(solnp_float));
      }
      else{
            c->ec = SOLNP_NULL;
      }

      if(nic > 0){
            c->ic = (solnp_float*)solnp_malloc(nic*sizeof(solnp_float));
      }
      else{
            c->ic = SOLNP_NULL;
      }

      c->cost = func;
      return c;
}


void set_default_settings(SOLNPSettings *stgs)
{
      stgs->rho = 1;
      stgs->max_iter = 50;
      stgs->min_iter = 10;
      stgs->delta = 1.;
      stgs->tol = 1e-4;
      stgs->tol_con = 1e-3;
      stgs->tol_restart = 1.;
      stgs->ls_time = 10;
      stgs->re_time = 5;
      stgs->delta_end = 1e-5;
      stgs->noise = 1;
      stgs->qpsolver = 1;
      stgs->k_i = 3.;
      stgs->k_r = 9;
      stgs->c_i = 30.;
      stgs->c_r = 10.;
}

void free_sol(SOLNPSol *sol)
{
      if(sol){
            if(sol->p){
                  solnp_free(sol->p);
            }
            if(sol->ic){
                  solnp_free(sol->ic);
            }
            if(sol->jh){
                  solnp_free(sol->jh);
            }
            if(sol->ch){
                  solnp_free(sol->ch);
            }
            if(sol->l){
                  solnp_free(sol->l);
            }
            if(sol->h){
                  solnp_free(sol->h);
            }

            solnp_free(sol);
      }
}
