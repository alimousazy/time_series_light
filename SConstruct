env = Environment(CCFLAGS='-pg')
env.Append(CPPPATH = ['/usr/local/include/'])
env.Append(LIBPATH = ['/usr/local/lib/'])

env.Program("time_series", ["main.c", "data_store.c", "time_convert.c", "tcp_server.c"], LIBS = ["pthread", "m", "rocksdb", "mill"])
