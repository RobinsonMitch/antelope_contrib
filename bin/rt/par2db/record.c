/**************************************************************************
 *
 *   utilities to write data/records to output database  
 *
 *
 ***************************************************************************/
#include "par2db.h"

extern int save_seed();
extern Steim *stinit();
 
void wrt_last_rec ( Db_buffer *buf )
{
    int result ;
    char *s ;
    if ( buf->params->datacode  == trSEED )  {
   
	    result = csteim (buf->steim, save_seed, 0, 0) ;
	    if ( result < 0 ) 
		die ( 1, "steim compression failed to write final bytes\n" ) ;
	    else if ( result > 0 ) {
		complain ( 1, "steim compression problems for %s_%s_%s at %s\n",
			    buf->net, buf->sta, buf->chan, s=strtime(buf->crnt_time) );
		free(s) ; 
	    }
	    if ( buf->steim != 0 ) {
		freesteim ( buf->steim ) ; 
		buf->steim = stinit ( buf ) ; 
	    }
     }

}

int record ( PktChannel *new , Db_buffer *buf) 
{
    double        crnt_time ;
    int           npts, ev_over = 0;
    int           doff, result ;
    int 	  nsamp_now, maxnsamp;
    int 	  *data;
    char          *s ;

    if (buf->db.record < 0) {
	if ( !new_dfile (buf, new->time ) ) 
	    die (0, "Couldn't add new record to database.\n" ) ; 
    } 
    npts = TIME2SAMP (buf->crnt_time, buf->samprate, new->time);
    if ( npts < 0 ) {
        complain ( 0, "%s_%s_%s : dropping data at time=%lf\n"
            "data starts before current time=%lf\n", 
            buf->net, buf->sta, buf->chan, new->time, buf->crnt_time );
        return -1 ; 
    }
 
    if ( !TRCONTIGUOUS(buf->stime, new->time, buf->samprate, buf->nsamp))  {

/*
fprintf( stderr, "gap in %s_%s.  prev_record over at %lf new starts at %lf\n",  
buf->sta, buf->chan, buf->crnt_time, new->time );
fflush(stderr);
*/

        wrt_last_rec ( buf ) ; 
        if ( !new_dbrecord (buf, new, buf->stime ) ) 
           die (0, "Couldn't add new record to database.\n" ) ; 
    }
    doff = 0;
    nsamp_now = 0;
    crnt_time = new->time;
    data =  (int *) new->data;

    while( new->nsamp > nsamp_now )  {
    
       maxnsamp = ( buf->tmax - crnt_time ) * buf->samprate;
       nsamp_now =  new->nsamp >= maxnsamp ? maxnsamp:new->nsamp;
       if( nsamp_now >= maxnsamp ) ev_over = 1; 
       else ev_over = 0;
/*
fprintf( stderr, "%s_%s nsamp %d will write %d at %lf\n",  
buf->sta, buf->chan, new->nsamp, nsamp_now, buf->crnt_time);
fflush(stderr);
 */   
       switch (buf->params->datacode) {

          case trSEED:
	      buf->nsamp += nsamp_now ; 
	      result = csteim (buf->steim, save_seed, data+doff, nsamp_now) ;
  
	      if ( result < 0 ) 
		    die (0, "steim compression failed\n" ) ;
	      else if ( result > 0 ) {
	          complain ( 1, "steim compression problems for %s_%s_%s at %s\n", 
			buf->net, buf->sta, buf->chan, s=strtime(crnt_time) ) ;
	          free(s) ; 
	      }
	      buf->crnt_time = ENDTIME(buf->stime, buf->samprate, buf->nsamp);

	  break;

          case trINT:
	      if ((npts = fwrite (data+doff, sizeof(int), 
		    nsamp_now, buf->file)) != nsamp_now) {
	            die (1, " write %d instead of %d samples to %s.\n",
		      npts, nsamp_now, buf->path ) ; 
	      }
	      if ( fflush(buf->file) != 0 ) 
	          die ( 1, "Can't flush %s\n", buf->path ) ; ;

	      buf->nsamp += nsamp_now ; 
	      buf->crnt_time = ENDTIME(buf->stime, buf->samprate, buf->nsamp);
	      if ( dbputv ( buf->db, 0, 
	          "nsamp", buf->nsamp,
	          "endtime", buf->crnt_time,
	          0 ) < 0 ) 
	          die (0, "Couldn't write to database\n") ; 

	  break;

        default:
 	   die (0, "Can't reccognize datacode %d\n", buf->params->datacode);
	   break;
       }
       doff += nsamp_now;
       crnt_time = ENDTIME(buf->stime, buf->samprate, buf->nsamp+1);
       if( ev_over )  {
  
/* 
fprintf( stderr, "%s_%s record is over. stime=%lf endtime= %lf\n",  buf->sta, buf->chan, buf->stime, buf->crnt_time);
fflush(stderr);

*/ 
 
         if( buf->params->datacode == trSEED )   wrt_last_rec ( buf ) ; 
         new_dfile( buf, crnt_time ); 

       }
       new->nsamp -= nsamp_now;
       nsamp_now = 0;
    }
 
    return 1;
}
