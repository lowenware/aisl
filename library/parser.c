#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include <cStuff/str-utils.h>
#include <aisl/http.h>

#include "parser.h"
#include "globals.h"
#include "stream.h"

/* length(multipart/form-data; boundary=) = 30 */
#define B_OFFSET 30

/* common HTTP headers */
static const char cCookie[]         = "Cookie";
static const char cContentType[]    = "Content-Type";
static const char cContentLength[]  = "Content-Length";
/*
static const char cConnection[]     = "Connection";
static const char cHost[]           = "Host";
static const char cUserAgent[]      = "User-Agent";
static const char cAccept[]         = "Accept";
static const char cAcceptLanguage[] = "Accept-Language";
static const char cAcceptEncoding[] = "Accept-Encoding";
*/

#define CLI_STREAM(x) ( ((stream_t)list_index(x->streams,x->istream)) )
/*
static void
debug(const char * label, char * buffer, int len)
{
  printf("<<< %s [", label);
  fwrite(buffer, 1, len, stdout);
  printf("]\n");
}
*/

static pair_t
pair_new(const char * key, int length)
{
  pair_t p = calloc(1, sizeof(struct pair));
  if (p)
  {
    p->key = str_ncopy(key, length);
    if (!p->key)
    {
      free(p);
      p = NULL;
    }
  }

  return p;
}


/* HTTP METHOD -------------------------------------------------------------- */

parser_status_t
parse_request_method(client_t cli, char ** b_ptr, int *b_len)
{
  char * cur = memchr(*b_ptr, ' ', *b_len);

  if (!cur)
  {
    if (*b_len < 8)
      return PARSER_HUNGRY;
    else
      return PARSER_FAILED;
  }

  int l = (int) (cur - *b_ptr);

  stream_t s = list_index(cli->streams, cli->istream);

  switch( l )
  {
    case 3:
      if (strncmp(*b_ptr, "GET", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_GET;
      else if (strncmp(*b_ptr, "PRI", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_PRI;
      else if (strncmp(*b_ptr, "PUT", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_PUT;
      else
        return PARSER_FAILED;
      break;
    case 4:
      if (strncmp(*b_ptr, "POST", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_POST;
      else if (strncmp(*b_ptr, "HEAD", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_HEAD;
      else
        return PARSER_FAILED;
      break;
    case 5:
      if (strncmp(*b_ptr, "TRACE", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_TRACE;
      else
        return PARSER_FAILED;
      break;
    case 6:
      if (strncmp(*b_ptr, "DELETE", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_DELETE;
      else
        return PARSER_FAILED;
      break;
    case 7:
      if (strncmp(*b_ptr, "OPTIONS", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_OPTIONS;
      else if (strncmp(*b_ptr, "CONNECT", l)==0)
        ASTREAM(s)->request_method = AISL_HTTP_CONNECT;
      else
        return PARSER_FAILED;
      break;
    default:
      return PARSER_FAILED;
  }

  *b_ptr += ++l;
  *b_len -= l; /* count method + space character */

  s->state = STREAM_REQUEST_PATH;

  return PARSER_PENDING;
}

/* HTTP REQUEST_URI and HOST ------------------------------------------------ */
static void
str_to_lower( char * src )
{
  while (*src)
  {
    *src = tolower(*src);
    src++;
  }
}

parser_status_t
parse_request_path(client_t cli, char ** b_ptr, int *b_len)
{
  parser_status_t result = PARSER_PENDING;
  stream_t s = list_index(cli->streams, cli->istream);

  int i;
  char * host = NULL,
       * path = NULL,
       * query = NULL;


  for ( i=0; i<*b_len; i++)
  {
    switch( (*b_ptr)[i] )
    {
      case ':':
        if (host) /* if host is set, we parse host and it could not contain : */
        {
          result = PARSER_FAILED;
          break;
        }
        else /* could be protocol separator */
        {
          if (i==5)
          {
            if (*b_len > 7 && strncmp(*b_ptr, "http://", 7)==0 ) /* protocol defined */
            {
              host = &(*b_ptr)[i+3];
              i+=2;
              continue;
            }

            result = PARSER_FAILED; /* something is wrong */
            break;
          }
        }

        continue;
      case '?':
        query = (*b_ptr) +i;
        continue;

      case ' ':
          if (! path) path = *b_ptr;
          if (query)
          {
            ASTREAM(s)->path  = str_ncopy(path, (uint32_t) (query-path));
            query++;
            ASTREAM(s)->query = str_ncopy(query, (uint32_t)(&(*b_ptr)[i]-query));
          }
          else
            ASTREAM(s)->path = str_ncopy(path, (uint32_t) (&(*b_ptr)[i]-path));

          *b_len -= ++i;
          *b_ptr += i;

          s->state = STREAM_REQUEST_PROTOCOL;

          return PARSER_PENDING;
        break;
      case '/':
        if (host)
        {
          /* debug("  > host", host, (int) (&(*b_ptr)[i] - host)); */
          pair_t p = malloc(sizeof(struct pair));
          if (p)
          {
            p->key = str_copy("host");
            p->value = str_ncopy(host, (uint32_t) (&(*b_ptr)[i] - host));
          }
          s->headers = list_new(AISL_MIN_HEADERS);

          list_append(s->headers, p);
          host = NULL;

          path = &(*b_ptr)[i];
        }
      default:
        continue;
    }
    break;

  }

  if (result == PARSER_PENDING) /* end space was not found */
    result = PARSER_HUNGRY;

  /*
  if (result == PARSER_HUNGRY && *b_len == gBuffer->size)
    result = PARSER_FAILED;*/ /* buffer is overloaded */

  return result;
}

/* HTTP VERSION ------------------------------------------------------------- */

parser_status_t
parse_request_protocol(client_t cli, char ** b_ptr, int *b_len)
{
  stream_t stream = CLI_STREAM(cli);
  /* HTTP/X.X = 8 characters minimal */

  if (*b_len < 8) return PARSER_HUNGRY;

  char * ptr = memchr(*b_ptr, '\n', *b_len);

  if (!ptr) return PARSER_HUNGRY;

  int l = (int) (ptr - *b_ptr);

  if (strncmp(*b_ptr, "HTTP/", 5)==0)
  {
    if (strncmp(&(*b_ptr)[5], "2.0", 3)==0) cli->protocol = AISL_HTTP_2_0; else
    if (strncmp(&(*b_ptr)[5], "1.1", 3)==0) cli->protocol = AISL_HTTP_1_1; else
    if (strncmp(&(*b_ptr)[5], "1.0", 3)==0) cli->protocol = AISL_HTTP_1_0; else
      return PARSER_FAILED;

    if ( (l==10 && *b_ptr[8]=='\r') || (l==9) )
    {
      /*r->version = str_ncopy(*b_ptr, 8); */

      *b_ptr += ++l;
      *b_len -=l;

      stream->state=STREAM_REQUEST_HEADER_KEY;



      aisl_raise_event(
        cli->server->owner,
        stream,
        AISL_STREAM_OPEN,
        ASTREAM(stream)->request_method,
        ASTREAM(stream)->path,
        ASTREAM(stream)->query
      );

      if (!stream->headers)
        stream->headers = list_new(AISL_MIN_HEADERS);
      else if (stream->headers->count == 1)
      {
        /* raise event for Host header */
        pair_t p = list_index(stream->headers, 0);

        aisl_raise_event(
          cli->server->owner,
          stream,
          AISL_STREAM_HEADER,
          p->key, p->value
        );

      }


      return PARSER_PENDING;
    }
  }

  return PARSER_FAILED;
}

/* HTTP HEADER KEY ---------------------------------------------------------- */

parser_status_t
parse_request_header_key(client_t cli, char ** b_ptr, int *b_len)
{
  stream_t stream = CLI_STREAM(cli);
  int l;

  /* check end of headers */
  switch (*b_ptr[0])
  {
    case '\r':
      if (*b_len>1)
      {
        if( strncmp(*b_ptr, "\r\n", 2)==0 )
        {
          l=2;
        }
        else
          return PARSER_FAILED;
      }
      else
        return PARSER_HUNGRY;
      break;

      case '\n':
        l=1;
        break;
      default:
        l=0;
  }

  if (l)
  {
    /* end of headers */
    *b_len -= l;
    *b_ptr += l;


    stream->state = STREAM_REQUEST_CONTENT;

    /* aisl_raise_event(cli->server->owner, CLI_STREAM(cli), AISL_STREAM_OPEN);
     * */

    return PARSER_PENDING;
  }

  /* header key */

  char * ptr = memchr(*b_ptr, ':', *b_len);

  if (!ptr)
  {
    /*
    if (*b_len == gBuffer->size)
      return PARSER_FAILED;
      */
    return PARSER_HUNGRY;
  }

  l = (int) (ptr-*b_ptr);


  pair_t ppp = pair_new(*b_ptr, l);

  str_to_lower(ppp->key);

  if (ppp)
    list_append(stream->headers, ppp);

  *b_len -= ++l;
  *b_ptr += l;

  stream->state=STREAM_REQUEST_HEADER_VALUE;

  return PARSER_PENDING;

}

/* HTTP HEADER VALUE -------------------------------------------------------- */

parser_status_t
parse_request_header_value(client_t cli, char ** b_ptr, int *b_len)
{
  stream_t stream = CLI_STREAM(cli);
  /* skip first space */

  if (*b_len)
  {
    if ((*b_ptr)[0]==' ')
    {
      (*b_ptr)++;
      (*b_len)--;
      if (*b_len == 0)
        return PARSER_HUNGRY;
    }
  }
  else
    return PARSER_HUNGRY;

  char * ptr = memchr(*b_ptr, '\n', *b_len);
  int l;

  l = (ptr) ? (int) (ptr-*b_ptr) : *b_len;

  uint32_t index = stream->headers->count -1;

  pair_t p = list_index(stream->headers, index);

  p->value = str_ncat(p->value, *b_ptr, (l && (*b_ptr)[l-1]=='\r') ? l-1 : l);

  *b_len -= ++l;
  *b_ptr += l;

  stream->state=STREAM_REQUEST_HEADER_KEY;

  /* todo: add limit for maximal header length */

  if (ptr)
  {
    if (str_cmpi(p->key, cCookie )==0)
    {
      /* parse cookies */
    }
    else if (str_cmpi(p->key, cContentType   )==0)
    {
      /* CLI(r)->c_type_index = index; */
    }
    else if (str_cmpi(p->key, cContentLength )==0)
    {
      stream->c_length = strtol(p->value, NULL, 0);
    }
      /* CLI(r)->c_length = strtol(p->value, NULL, 10); */

    aisl_raise_event(
      cli->server->owner,
      stream,
      AISL_STREAM_HEADER,
      p->key, p->value
    );

    return PARSER_PENDING;
  }
  else
    return PARSER_HUNGRY;

}



parser_status_t
parse_request_content(client_t cli, char ** b_ptr, int *b_len)
{
  stream_t stream = CLI_STREAM(cli);
  /*
  fprintf(stdout, "AISL [%d]> ", CLI_STREAM(cli)->c_length);
  fwrite(*b_ptr, 1, *b_len, stdout);
  fprintf(stdout, "\n<");
  */

  if (stream->c_length)
  {
    int l = *b_len;

    aisl_raise_event(
      cli->server->owner,
      stream,
      AISL_STREAM_INPUT,
      *b_ptr,
      l
    );

    *b_ptr += l;
    *b_len = 0;
    stream->c_length -= l;
  }
  else
    goto request_ready;

  if ( stream->c_length == 0 )
  {
    request_ready:
      stream->state=STREAM_REQUEST_READY;
       aisl_raise_event( cli->server->owner,  stream, AISL_STREAM_REQUEST );
      return PARSER_FINISHED;
  }
  else
    return PARSER_HUNGRY;
}
