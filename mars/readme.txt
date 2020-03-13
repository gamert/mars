

2020/3/13: vs2017 2015 都可以compile 6个project
			vs2019 可以使用2017 2015的SDK来编译。
			但自vs2017的15.9版本开始，不存在#include <thr/threads.h>，导致无法编译
			一种办法是使用老的thr/threads.h来替换，但需要整个目录，因为xthreads.h已经有新的接口和定义变动导致编译错误；
			