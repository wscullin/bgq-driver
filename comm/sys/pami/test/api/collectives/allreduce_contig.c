/* begin_generated_IBM_copyright_prolog                             */
/*                                                                  */
/* This is an automatically generated copyright prolog.             */
/* After initializing,  DO NOT MODIFY OR MOVE                       */
/*  --------------------------------------------------------------- */
/* Licensed Materials - Property of IBM                             */
/* Blue Gene/Q 5765-PER 5765-PRP                                    */
/*                                                                  */
/* (C) Copyright IBM Corp. 2011, 2012 All Rights Reserved           */
/* US Government Users Restricted Rights -                          */
/* Use, duplication, or disclosure restricted                       */
/* by GSA ADP Schedule Contract with IBM Corp.                      */
/*                                                                  */
/*  --------------------------------------------------------------- */
/*                                                                  */
/* end_generated_IBM_copyright_prolog                               */
/**
 * \file test/api/collectives/allreduce_contig.c
 * \brief Simple Allreduce on world geometry with contiguous datatypes
 */

/* Use arg -h or see setup_env() for environment variable overrides  */

#include "../pami_util.h"

int main(int argc, char*argv[])
{
  pami_client_t        client;
  pami_context_t      *context;
  pami_task_t          task_id, task_zero = 0;
  size_t               num_tasks;
  pami_geometry_t      world_geometry;

  /* Barrier variables */
  size_t               barrier_num_algorithm[2];
  pami_algorithm_t    *bar_always_works_algo = NULL;
  pami_metadata_t     *bar_always_works_md   = NULL;
  pami_algorithm_t    *bar_must_query_algo   = NULL;
  pami_metadata_t     *bar_must_query_md     = NULL;
  pami_xfer_type_t     barrier_xfer = PAMI_XFER_BARRIER;
  volatile unsigned    bar_poll_flag = 0;

  /* Allreduce variables */
  size_t               allreduce_num_algorithm[2];
  pami_algorithm_t    *next_algo = NULL;
  pami_metadata_t     *next_md= NULL;
  pami_algorithm_t    *allreduce_always_works_algo = NULL;
  pami_metadata_t     *allreduce_always_works_md = NULL;
  pami_algorithm_t    *allreduce_must_query_algo = NULL;
  pami_metadata_t     *allreduce_must_query_md = NULL;
  pami_xfer_type_t     allreduce_xfer = PAMI_XFER_ALLREDUCE;
  volatile unsigned    allreduce_poll_flag = 0;

  int                  i, j, nalg = 0, total_alg;
  double               ti, tf, usec;
  pami_xfer_t          barrier;
  pami_xfer_t          allreduce;

  /* Process environment variables and setup globals */
  if(argc > 1 && argv[1][0] == '-' && (argv[1][1] == 'h' || argv[1][1] == 'H') ) setup_env_internal(1);
  else setup_env();

  assert(gNum_contexts > 0);
  context = (pami_context_t*)malloc(sizeof(pami_context_t) * gNum_contexts);

  /*  Allocate buffer(s) */
  int err = 0;
  void* sbuf = NULL;
  err = posix_memalign(&sbuf, 128, gMax_byte_count + gBuffer_offset);
  assert(err == 0);
  sbuf = (char*)sbuf + gBuffer_offset;
  void* rbuf = NULL;
  err = posix_memalign(&rbuf, 128, gMax_byte_count + gBuffer_offset);
  assert(err == 0);
  rbuf = (char*)rbuf + gBuffer_offset;

  /*  Initialize PAMI */
  int rc = pami_init(&client,        /* Client             */
                     context,        /* Context            */
                     NULL,           /* Clientname=default */
                     &gNum_contexts, /* gNum_contexts       */
                     NULL,           /* null configuration */
                     0,              /* no configuration   */
                     &task_id,       /* task id            */
                     &num_tasks);    /* number of tasks    */

  if (rc != PAMI_SUCCESS)
    return 1;

  /*  Query the world geometry for barrier algorithms */
  rc |= query_geometry_world(client,
                             context[0],
                             &world_geometry,
                             barrier_xfer,
                             barrier_num_algorithm,
                             &bar_always_works_algo,
                             &bar_always_works_md,
                             &bar_must_query_algo,
                             &bar_must_query_md);

  if (rc != PAMI_SUCCESS)
    return 1;

  barrier.cb_done   = cb_done;
  barrier.cookie    = (void*) & bar_poll_flag;
  barrier.algorithm = bar_always_works_algo[0];

  unsigned iContext = 0;

  for (; iContext < gNum_contexts; ++iContext)
  {

    if (task_id == 0)
      printf("# Context: %u\n", iContext);

    int o;
    for(o = -1; o <= gOptimize ; o++) /* -1 = default, 0 = de-optimize, 1 = optimize */
    {

      pami_configuration_t configuration[1];
      configuration[0].name = PAMI_GEOMETRY_OPTIMIZE;
      configuration[0].value.intval = o; /* de/optimize */
      if(o == -1) ; /* skip update, use defaults */
      else
        rc |= update_geometry(client,
                              context[iContext],
                              world_geometry,
                              configuration,
                              1);

      if (rc != PAMI_SUCCESS)
        return 1;

    /*  Query the world geometry for allreduce algorithms */
    rc |= query_geometry_world(client,
                               context[iContext],
                               &world_geometry,
                               allreduce_xfer,
                               allreduce_num_algorithm,
                               &allreduce_always_works_algo,
                               &allreduce_always_works_md,
                               &allreduce_must_query_algo,
                               &allreduce_must_query_md);

    if (rc != PAMI_SUCCESS)
      return 1;
    total_alg = allreduce_num_algorithm[0]+allreduce_num_algorithm[1];
    for (nalg = 0; nalg < total_alg; nalg++)
    {
      metadata_result_t result = {0};
      unsigned query_protocol;
      if(nalg < allreduce_num_algorithm[0])
      {  
        query_protocol = 0;
        next_algo = &allreduce_always_works_algo[nalg];
        next_md  = &allreduce_always_works_md[nalg];
      }
      else
      {  
        query_protocol = 1;
        next_algo = &allreduce_must_query_algo[nalg-allreduce_num_algorithm[0]];
        next_md  = &allreduce_must_query_md[nalg-allreduce_num_algorithm[0]];
      }
      if (task_id == task_zero)
      {
        printf("# Allreduce Bandwidth Test(size:%zu) -- context = %d, optimize = %d, protocol: %s, Metadata: range %zu <-> %zd, mask %#X\n",num_tasks,
               iContext, o, next_md->name,
               next_md->range_lo,(ssize_t)next_md->range_hi,
               next_md->check_correct.bitmask_correct);
        printf("# Size(bytes)      iterations     bytes/sec      usec\n");
        printf("# -----------      -----------    -----------    ---------\n");
      }

      if (((strstr(next_md->name, gSelected) == NULL) && gSelector) ||
          ((strstr(next_md->name, gSelected) != NULL) && !gSelector))  continue;

      gProtocolName = next_md->name;

      unsigned checkrequired = next_md->check_correct.values.checkrequired; /*must query every time */
      assert(!checkrequired || next_md->check_fn); /* must have function if checkrequired. */

      allreduce.cb_done   = cb_done;
      allreduce.cookie    = (void*) & allreduce_poll_flag;
      allreduce.algorithm = *next_algo;
      allreduce.cmd.xfer_allreduce.rtype     = PAMI_TYPE_BYTE;
      allreduce.cmd.xfer_allreduce.rtypecount = 0;

      int op, dt;

      for (dt = 0; dt < dt_count; dt++)
      {
        for (op = 0; op < op_count; op++)
        {
          if (gValidTable[op][dt])
          {
            if (task_id == task_zero)
              printf("Running Allreduce: %s, %s\n", dt_array_str[dt], op_array_str[op]);

            for ( i = gMin_byte_count? MAX(1,gMin_byte_count/get_type_size(dt_array[dt])) : 0; /*clumsy, only want 0 if hardcoded to 0, othersize min 1 */
                  i <= gMax_byte_count/get_type_size(dt_array[dt]); 
                  i = i ? i*2 : 1 /* handle zero min */)
            {
              size_t sz=get_type_size(dt_array[dt]);
              size_t  dataSent = i * sz;
              int niter;

              if (dataSent < CUTOFF)
                niter = gNiterlat;
              else
                niter = NITERBW;

              allreduce.cmd.xfer_allreduce.stypecount = i;
              allreduce.cmd.xfer_allreduce.rtypecount = i;
              allreduce.cmd.xfer_allreduce.rtype = dt_array[dt];
              allreduce.cmd.xfer_allreduce.stype = dt_array[dt];
              allreduce.cmd.xfer_allreduce.op    = op_array[op];

              if(query_protocol)
              {  
                size_t sz=get_type_size(dt_array[dt])*i;
                /* Must initialize all of cmd for metadata */
                allreduce.cmd.xfer_allreduce.sndbuf    = sbuf;
                allreduce.cmd.xfer_allreduce.rcvbuf    = rbuf;
                result = check_metadata(*next_md,
                                      allreduce,
                                      dt_array[dt],
                                      sz, /* metadata uses bytes i, */
                                      sbuf,
                                      dt_array[dt],
                                      sz,
                                      rbuf);
                if (next_md->check_correct.values.nonlocal)
                {
                  /* \note We currently ignore check_correct.values.nonlocal
                     because these tests should not have nonlocal differences (so far). */
                  result.check.nonlocal = 0;
                }

                if (result.bitmask) continue;
              }

              /* Do one 'in-place' collective (if supported) and validate it */
              if(gTestMpiInPlace && (!query_protocol || (query_protocol && next_md->check_correct.values.inplace)))
              {
                allreduce.cmd.xfer_allreduce.sndbuf    = PAMI_IN_PLACE;
                allreduce.cmd.xfer_allreduce.rcvbuf    = sbuf;
                if (checkrequired) /* must query every time */
                {
                  result = next_md->check_fn(&allreduce);
                  if (result.bitmask) continue;
                }
                reduce_initialize_sndbuf (sbuf, i, op, dt, task_id, num_tasks);
                blocking_coll(context[iContext], &allreduce, &allreduce_poll_flag);
                int rc_check;
                rc |= rc_check = reduce_check_rcvbuf (sbuf, i, op, dt, task_id, num_tasks);
                if (rc_check) fprintf(stderr, "%s FAILED IN PLACE validation on %s/%s\n", gProtocolName, dt_array_str[dt], op_array_str[op]);
              }
              else if(gTestMpiInPlace && gVerbose>=2 && (task_id == task_zero)) printf("%s does not support IN PLACE buffering\n", gProtocolName);

              /* Iterate (and time) with separate buffers, not in-place */
              allreduce.cmd.xfer_allreduce.sndbuf    = i==0 && gAllowNullSendBuffer ?NULL:sbuf;
              allreduce.cmd.xfer_allreduce.rcvbuf    = rbuf;
              reduce_initialize_sndbuf (sbuf, i, op, dt, task_id, num_tasks);
              memset(rbuf, 0xFF, dataSent);

              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);
              ti = timer();

              for (j = 0; j < niter; j++)
              {
                if (checkrequired) /* must query every time */
                {
                  result = next_md->check_fn(&allreduce);
                  if (result.bitmask) continue;
                }
                blocking_coll(context[iContext], &allreduce, &allreduce_poll_flag);
              }

              tf = timer();
              /* We aren't testing barrier itself, so use context 0. */
              blocking_coll(context[0], &barrier, &bar_poll_flag);

              int rc_check;
              rc |= rc_check = reduce_check_rcvbuf (rbuf, i, op, dt, task_id, num_tasks);

              if (rc_check) fprintf(stderr, "%s FAILED validation on %s/%s\n", gProtocolName, dt_array_str[dt], op_array_str[op]);

              usec = (tf - ti) / (double)niter;

              if (task_id == task_zero)
              {
                printf("  %11lld %16d %14.1f %12.2f\n",
                       (long long)dataSent,
                       niter,
                       (double)1e6*(double)dataSent / (double)usec,
                       usec);
                fflush(stdout);
              }
              /* Iteration based validations.  Write something different (and validate) every iteration*/
              for (j = 0; j < niter; j++)
              {
                if (checkrequired) /* must query every time */
                {
                  result = next_md->check_fn(&allreduce);
                  if (result.bitmask) continue;
                }
                /* Iteration data validation */
                reduce_initialize_sndbuf_iter(sbuf, i, op, dt, j);
                blocking_coll(context[iContext], &allreduce, &allreduce_poll_flag);
                /* Iteration data validation */
                int rc_check;
                rc |= rc_check = reduce_check_rcvbuf_iter (rbuf, i, op, dt, j, num_tasks);
                if (rc_check) 
                  fprintf(stderr, "%s(%d) FAILED validation on %s/%s on iteration %d\n", gProtocolName, i, dt_array_str[dt],op_array_str[op],j);
              }
            }
          }
        }
      }
    }

    free(allreduce_always_works_algo);
    free(allreduce_always_works_md);
    free(allreduce_must_query_algo);
    free(allreduce_must_query_md);

  } /* optimize loop */
  } /*for(unsigned iContext = 0; iContext < gNum_contexts; ++iContexts)*/

  free(bar_always_works_algo);
  free(bar_always_works_md);
  free(bar_must_query_algo);
  free(bar_must_query_md);



  sbuf = (char*)sbuf - gBuffer_offset;
  free(sbuf);
  rbuf = (char*)rbuf - gBuffer_offset;
  free(rbuf);

  rc |= pami_shutdown(&client, context, &gNum_contexts);
  return rc;
}
