env = Environment(CCFLAGS='-g')
env.Append(CPPPATH = ['/usr/local/include/'])
env.Append(LIBPATH = ['/usr/local/lib/'])

env.Program("time_series", ["main.c", "circular_cache.c", "data_store.c", "time_convert.c", "tcp_server.c"], LIBS = ["pthread", "m", "rocksdb", "mill"])
