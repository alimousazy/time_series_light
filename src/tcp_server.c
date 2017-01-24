#include "tcp_server.h"

struct tcp_server *tcp_server_create(int port) {	
	struct tcp_server *server; 
	if( port < 0 ) {
		return NULL;
	}
	server = malloc(sizeof(struct tcp_server));
	if (!server)
		return NULL;
	ipaddr addr = iplocal(NULL, port, 0);
  server->sk   = tcplisten(addr, 10);
	if (!server->sk) {
		free(server);
		return NULL;
	}
	return server;
}

int tcp_server_accept(struct tcp_server* server) {
	while (1) {
		tcpsock as = tcpaccept(server->sk, -1);
    if(!as)
       return -1;
		go(tcp_process_data(server, as));
	}
}


static int write_data(struct tcp_server* server, char **parts) {
  const short TIME_POS = 1;
  const short NAME_POS = 2;
  const short VALUE_POS = 3;
  time_t m_time;
  float value;
  char *endptr;
  int error;
  m_time = strtol(parts[TIME_POS], &endptr, 10);
  if (endptr && *endptr != '\0') {
    return -1;
  }
  value = strtof(parts[VALUE_POS], &endptr);
  if (endptr && *endptr != '\n') {
    return -1;
  }
  error = store_dp(server->ds, parts[NAME_POS], m_time, value);
  if(error < 0) {
    return -1;
  }
  return 0;
}

static int read_data(struct tcp_server* server, char **parts, tcpsock sk) {
  const short TIME_START_POS = 1;
  const short TIME_END_POS = 2;
  const short NAME_POS = 3;
  time_t s_time;
  time_t e_time;
  char *endptr;
  int error;
  char *pos;
  struct range_query_result r;
  char outbuff[200];
  char *start_data = "{ \"data\" : [";
  char *end_data = " ]}";
  time_t start_date, t_point, i;
  s_time = strtol(parts[TIME_START_POS], &endptr, 10);
  if (endptr && *endptr != '\0') {
    return -1;
  }

  e_time = strtol(parts[TIME_END_POS], &endptr, 10);
  if (endptr && *endptr != '\0') {
    return -1;
  }

  pos = strstr(parts[NAME_POS], "\n");
  if (pos) {
    *pos = '\0';
  }

  tcpsend(sk, start_data, strlen(start_data), -1);
  for(t_point = init_ds_iter(s_time); t_point <= e_time; t_point = incr_ds_iter(t_point, SHARD_SIZE)) {
    time_t pos = 0;
    r = ds_current(server->ds, parts[NAME_POS], t_point, &error);
    pos = r.start_date;
    char p_template[] = "[%ld000, %f],";
    if(s_time >= r.start_date && s_time <= r.start_date +  r.shard_size) {
      pos = s_time;
    }

    for(i = pos; i <= e_time && (i - r.start_date) < r.shard_size; i++) {
      if(i == e_time) {
        p_template[strlen(p_template) - 1] = '\0';
      }
      sprintf(outbuff, p_template, i,  r.points[i - r.start_date]);
      tcpsend(sk, outbuff, strlen(outbuff), -1);
    }
  }
  tcpsend(sk, end_data, strlen(end_data), -1);
  tcpflush(sk, -1);
  free_range_query(&r); 
  return 0;
}


coroutine void tcp_process_data(struct tcp_server* server, tcpsock sk) {
  int64_t deadline = now() + tcp_server_timeout;
	char inbuf[256];
  char *error = "Error happend";
  char *success = "done";
  int count = 0;
	while (1) {
    char *parts[3];   
    int num_process = 0;
		size_t sz = tcprecvuntil(sk, inbuf, sizeof(inbuf) - 1, "\n", 1, deadline);
		inbuf[sz] = '\0';
		if(errno != 0)
 	 	  goto cleanup;
    num_process = util_process_line(inbuf, parts, ":", 4);
    switch(inbuf[0]) {
      case 'w':
        if (num_process != 4) {
          continue;
        }
        if(write_data(server, parts) != -1) {
           tcpsend(sk, success, strlen(success), -1);
        } else {
           tcpsend(sk, error, strlen(error), -1);
        };
        tcpflush(sk, -1);
        break;
      break;
      case 'r':
        if (num_process != 4) {
          continue;
        }
        if (read_data(server, parts, sk) == -1) {
        }
        goto cleanup;
      break;
    }
    count++;
    if ( count == TCP_SERVER_YIELD_EVERY ) {
      count = 0;
      yield();
    }
	}
	cleanup:
		tcpclose(sk);
}

void tcp_server_set_ds(struct tcp_server *server, struct data_store *ds) {
  assert(ds);
  assert(server);
  server->ds = ds;
}

void tcp_server_cleanup(struct tcp_server *server)
{
	tcpclose(server->sk);
	free(server);
}
