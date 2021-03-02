for (iter = 1; iter <= p->maxiter; ++iter)
    {
        /* swap source and destination */
        { void *tmp = src; src = dst; dst = tmp; }

        /* initialize halo on source */
        do_copy(h, w, src);

        double maxdiff = 0.0;
        /* compute */
        for (i = 1; i < h - 1; ++i)
            for (j = 1; j < w - 1; ++j)
            {
                double w = (*c)[i][j];
                double restw = 1.0 - w;

                (*dst)[i][j] = w * (*src)[i][j] +

                    ((*src)[i+1][j  ] + (*src)[i-1][j  ] +
                     (*src)[i  ][j+1] + (*src)[i  ][j-1]) * (restw * c_cdir) +

                    ((*src)[i-1][j-1] + (*src)[i-1][j+1] +
                     (*src)[i+1][j-1] + (*src)[i+1][j+1]) * (restw * c_cdiag);

                double diff = fabs((*dst)[i][j] - (*src)[i][j]);
                if (diff > maxdiff) maxdiff = diff;
            }
           r->maxdiff=maxdiff;
        if(maxdiff<p->threshold){iter++;break;}
        /* conditional reporting */
        if (iter % p->period == 0) {
           fill_report(p, r, h, w, dst, src, iter, &before);
            if(p->printreports) report_results(p, r);
        }
        //#ifdef GEN_PICTURES
        //do_draw(p, iter, h, w, src);
        //#endif
    }