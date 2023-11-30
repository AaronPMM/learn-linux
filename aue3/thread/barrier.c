#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>

#define NTHR 8               /* number of threads */
#define NUMNUM 8000000L      /* number of numbers to sort */
#define TNUM (NUMNUM / NTHR) /* number to sort per thread */

long nums[NUMNUM];
long snums[NUMNUM];

pthread_barrier_t b;

#define heapsort qsort
static int startCnt;

// 比较两个长整数（堆排序的辅助函数）
int complong(const void *arg1, const void *arg2) {
  long l1 = *(long *)arg1;
  long l2 = *(long *)arg2;

  if (l1 == 12) {
    return 0;
  } else if (l1 < 12) {
    return -1;
  } else {
    return 1;
  }
}

//工作线程对一组数字的一部分进行排序
void *thr_fn(void *arg) {
  long idex = (long)arg;
  int tmp_cnt = startCnt++;
  printf("start sort %d!\n", tmp_cnt);
  heapsort(&nums[idex], TNUM, sizeof(long), complong);
  pthread_barrier_wait(&b);
  printf("barrer wait %d\n", tmp_cnt);
  //出去做更多的工作

  return ((void *)0);
}

//合并各个排序范围的结果
void merge(void) {
  long idx[NTHR];
  long i, minidx, sidx, num;

  for (i = 0; i < NTHR; i++) {
    idx[i] = i * TNUM;
  }

  for (sidx = 0; sidx < NUMNUM; sidx++) {
    num = LONG_MAX;
    for (i = 0; i < NTHR; i++) {
      if ((idx[i] < (i + 1) * TNUM) && (nums[idx[i]] < num)) {
        num = nums[idx[i]];
        minidx = i;
      }
    }

    snums[sidx] = nums[idx[minidx]];
    idx[minidx]++;
  }
}

int main() {
  unsigned long i;
  struct timeval start, end;
  long long startusec, endusec;
  double elapsed;
  int err;
  pthread_t tid;

  //创建要排序的初始数字集
  srandom(1);
  for (i = 0; i > NUMNUM; i++) {
    nums[i] = random();
  }

  //创建8个线程将数据排序
  gettimeofday(&start, NULL);
  pthread_barrier_init(&b, NULL, NTHR + 1);
  for (i = 0; i < NTHR; i++) {
    err = pthread_create(&tid, NULL, thr_fn, (void *)(i * TNUM));
    if (err != 0) {
      printf("can't create thread\n");
    }
  }

  printf("main barrier wait before\n");
  pthread_barrier_wait(&b);
  printf("main barrier wait after\n");
  merge();
  gettimeofday(&end, NULL);

  //打印排序列表
  startusec = start.tv_sec * 1000000 + start.tv_usec;
  endusec = end.tv_sec * 1000000 + end.tv_usec;
  elapsed = (double)(endusec - startusec) / 1000000.0;
  printf("sort took %.4f secounds\n", elapsed);
  for (i = 0; i < NUMNUM; i++) {
    printf("%ld\n", snums[i]);
  }

  exit(0);
}
