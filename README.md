# CPU_Scheduler
## Ⅰ 서론

### 1. 구현 환경 및 언어

   구현 환경: Linux

   언어: C Language

### 2. CPU scheduling

   CPU scheduling이란 multi programming을 가능하게 하는 OS의 동작 기법이다. 각 프로세스에 CPU를 적절히 할당하여 시스템의 성능을 향상 시키며 CPU 이용률의 극대화를 목표로 한다.

   따라서 CPU 스케줄링 알고리즘이란 ‘어느 프로세스를 우선하여 CPU를 할당할 것인가’를 정하는 기준이 되며 크게 비선점형과 선점형의 2가지로 구분할 수 있다. 비선점형 스케쥴링에서는 실행 중인 프로세스가 종료되거나 입출력 요구가 발생하지 않는 이상 계속된 실행을 보장하며 FCFS(First Come First Served Scheduling) 스케줄링, SJF(Shortest Job First) 스케줄링, HRRN(Highest Response Ratio Next) 스케줄링, 비선점형 Priority 스케쥴링 등의 알고리즘이 있다. 반면 선점형 스케줄링에서는 우선 순위 등의 조건에 따라 다른 프로세스가 실행 중인 프로세스를 중지하고 CPU를 강제로 점유할 수 있으며 그 방법으로는 RR(Round Robin)스케줄링, SRTF(Shortest Remaining-Time First) 스케줄링, 다단계 큐 스케줄링, 다단계 피드백 큐 스케줄링, RM (Rate Monotonic) 스케줄링, EDF(Earliest Deadline First) 스케줄링, 선점형 Priority 스케쥴링 등이 있다.

   이 글에서는 FCFS, SJF, SRTF, 비선점형 Priority, 선점형 Priority, RR 총 여섯 개의 스케쥴링 알고리즘을 각각 설명하고 AWT(Average Waiting Time), ATT(Average Turnaround Time) 등의 비교를 통해 알고리즘들 간 성능 분석을 진행한다.  



## Ⅱ 본론

### 1. 기본 알고리즘 설명

#### First Come, First Served(FCFS)

FCFS 방식은 먼저 도착한 프로세스를 먼저 실행하는 방법이다. non-preemptive 방식 알고리즘으로, 수행중인  프로세스는 입출력 요구가 발생하지 않는 이상 종료될 때까지 수행 되며, 다른 프로세스들은 준비 큐에서 기다린다. 비교적 간단하지만 작업량이 많은 프로세스가 CPU를 우선 차지해서 다른 프로세스들이 오랜시간 기다려야 하는 상황인 호위 상태(convoy effect)가 발생할 수 있다.

#### Shortest Job First(SJF) & Shortest Remaining Time First(SRTF)

SJF는 남은 CPU 수행 시간이 가장 짧은 프로세스에게 CPU를 할당해주는 방법으로 평균 대기시간을 최소로 하는 것을 목표로 한다. 효율적이지만 프로세스의 남은 CPU burst time 이라는 future knowledge 를 요구하기 때문에 실제로 적용하기에는 한계가 있다. 또한 요구 시간이 긴 프로세스에 대해 starvation 현상이 발생할 수 있다. SJF는 비선점형와 선점형 모두에 적용될 수 있는데, 선점형 SJF 스케줄링이 바로 SRTF 스케줄링이다. 해당 스케줄링 알고리즘은 현재 수행되고 있는 프로세스의 remaining CPU burst time보다 작은 remaining CPU burst time을 가진 프로세스가 있다면 현재 프로세스를 중단하고 해당 프로세스에 CPU를 할당 한다.

#### Preemptive Priority & Non Preemptive Priority

   Priority 방식은 priority가 가장 높은 프로세스에게 CPU를 할당해주는 방법으로 우선 순위가 낮은 프로세스에 대해 starvation 현상이 발생할 수 있다. 이 방식 역시 비선점형와 선점형 모두에 적용될 수 있는데 비선점형의 경우 Ready Queue에 현재 CPU가 할당 된 프로세스보다 높은 우선 순위를 가진 프로세스가 있더라도 현재의 프로세스를 마저 수행하고, 해당 상황에서 선점형의 경우에는 현재 프로세스를 중단 시키고 우선 순위가 높은 프로세스에 CPU를 할당한다.

#### Round Robin(RR)

   Round Robin방식은 FCFS와 비슷하지만 preemption이 추가된 형태로, 일정한 time quantum(q)값을 설정하여 그 만큼의 시간이 지나면 현재 수행을 중단하고 무조건 다음 프로세스에 CPU를 할당해주는 방식이다. 따라서 각 프로세스는 (n-1)q 시간 안에 수행을 보장 받을 수 있다. 하지만 time quantum이 너무 크다면 FCFS 스케쥴링 알고리즘과 같아질 것이고, 너무 작다면 잦은 context switch로 인해 좋은 퍼포먼스를 기대하기 어려우므로 적절한 time quantum 값을 설정해야 한다.



### 2. Scheduler 시스템 구성도

   해당 시스템의 흐름은 다음과 같다. 
   
1. n개의 랜덤 프로세스 생성.
	
2. 스케줄링 방법 선택. (FCFS, SJF, SRTF, preemptive Priority, non preemptive Priority, RR 중 하나)

3. 해당 스케줄링 진행.

4. evaluation (ATT, AWT)

5. 2-4 반복

![scheduler](https://github.com/sujinnaljin/CPU_Scheduler/tree/master/images/scheduler.png)


### 3. 시뮬레이터의 모듈

#### 주요 구조체 및 변수

Process의 정보 담고 있다. Process의 id, 남은 CPU 및 I/O burst time, ready Queue에 들어온 시간, 프로세스 수행이 끝난 시간, 연속으로 CPU에 할당되어 수행된 시간, 우선 순위 등을 나타낸다.

```c
typedef struct {
	int process_id;
	int cpu_burst_time;
	int io_burst_time;
	int arrival_time;
	int terminate_time;
	int continued_time;
	int priority;
} Process;
```



Process의 Array와 해당 priority queue에 담긴 프로세스 개수의 정보 담고 있다.

```c
typedef struct priority_queue {
	Process heap[MAX_SIZE];
	int size;
}priority_queue;
```



현재까지 수행 된 시간을 나타낸다. 턴마다 1씩 증가한다.

```c
int currentTime = 0;
```



CPU가 다른 프로세스에 의해 수행되고 있는지 파악할 수 있다.

```c
bool isCpuBusy = false;
```



I/O가 다른 프로세스에 의해 수행되고 있는지 파악할 수 있다.

```c
bool  isIOBusy = false;
```



현재 CPU를 수행하고 있는 프로세스

```c
Process runningCPUProcess;
```



현재 I/O를 수행하고 있는 프로세스

```c
Process runningIOProccess;
```



#### 주요 함수

queue에 *process*를 집어 넣거나 빼낸다*.* 그 과정에 인자로 넘겨 받은 스케줄링 메소드를 바탕으로 큐를 정렬한다*.* 이때 정렬 기준이 되는 *Process*의 속성은 *FCFS, RR* 에서는 *arrival_time, SJF, STRF* 에서는 *cpu_burst_time, NonPreempviePriority, PreemtivePriority* 에서는 *priority*이다.

```c
int pq_push(priority_queue *q, Process value, scheduling_method scheduling_method);
Process pq_pop(priority_queue *q, scheduling_method scheduling_method);
```



cpu_burst_time, *io_burst_time, arrival_time, priority* 등의 속성이 정해진 최대값 내에서 *random*으로 할당된 프로세스를 생성하는 함수이다. cpu_burst_time <= 1 일때는 *io_burst_time* 이 남아 있으면 안되므로 *0*을 할당한다.

```c
Process createRandomProcess(int uniqueId);
```



알고리즘 성능 체크하고 테이블을 출력한다. terminate_time - arrival_time - cpu_burst_time 을 통해 각 프로세스의 waiting Time을 구하고, terminate_time - arrival_time 을 통해 turnaroundTime을 구한다.

```c
void evaluateAlgorithm(priority_queue *terminatedQueue);
```



메인 스케쥴링 알고리즘 관련된 함수이다. 다음 턴에 CPU를 할당받을 Process를 결정하고 I/O를 수행중이거나, 수행해야하는 프로세스의 동작을 돕는다. CPU나 I/O의 idle 상태를 판단하기도 한다.

```c
void doScheduling(scheduling_method sch_method, priority_queue *jobQueue, priority_queue *readyQueue, priority_queue *waitingQueue, priority_queue *terminatedQueue);
```

doScheduling 구현부의 메인 로직은 다음과 같다.

```c
while (terminatedQueue의 사이즈가 프로세스의 전체 개수만큼 도달하지 않았을때) {
   
1. 생성된 프로세스 arrivalTime 체크해서 reday 큐로 할당한다.

if (isCpuBusy) {

switch (스케줄링 방법) {

2.만약 CPU가 Busy 상태면 스케줄링 방법에 따라 runningProcess를 결정한다

case FCFS : case SJF : case NonPreempviePriority :

하던 작업을 마저 수행한다. cpu가 busy한 상태라면 이전에 수행하던 프로세스가 전역변수 runningCPUProcess 에 할당되어 있을 것이다.

case PreemtivePriority :

현재 진행 프로세스의 우선순위가 readyQueue에 있는 것보다 낮다면, 원래 프로세스 레디큐에 집어넣고 runningProcess를 우선 순위가 높은 것으로 바꾼다

case STRF :

현재 진행 프로세스의 남은 cpu burst time이 readyQueue에 있는 것보다 길다면, 원래 프로세스 레디큐에 집어넣고 runningProcess를 남은 cpu burst time이 짧은 것으로 바꾼다

case RR:

만약에 레디큐에 다른 프로세스가 있는데, 현재 수행되고 있는 프로세스가 time quantum 이상 수행 됐다면, 원래 프로세스 레디큐에 집어넣고 runningProcess를 레디 큐에서 꺼낸 프로세스로 바꾼다. 이때 프로세스를 레디큐에 집어넣을 때 해당 process의 continued_time(지속 시간)은 0으로 초기화 한 뒤 넣어야 한다.

}// switch            

doCPUOperation() 3. 위의 코드에 의해 결정된 runningCPUProcess의 CPU 작업을 수행한다.

} else {

4. cpu not busy 상태면 readyQueue에서 새로운 프로세스를 꺼내서 cpu에 할당해줘야 한다.

5. 그 전에 readyQueue 비어있는 상태인지 체크해야 하는데 비어있다면 cpu는 idle 상태이다.

}

if (isIOBusy) {

doIOOperation() 6. 만약 I/O가 busy 하면 하던 작업(runningIOProcess)을 마저 수행한다.

} else {

7. IO not busy 상태면 waitingQueue에서 꺼내서 IO 할당해줘야 한다.

8. 그 전에 waitingQueue 비어있는 상태인지 체크해야 하는데 비어있다면 IO는 dle 상태이다.

}

currentTime++; 9.cpu랑 IO 체크 작업 다 했으니까 time 올려주고 해당 작업 수행을 반복한다.

} //while
```



특정 프로세스가 I/O를 수행하게 되었을때 실행되는 함수이다. 들어온 process의 io_burst_time을 1만큼 줄이고 다음에도 해당 프로세스가 I/O operation을 수행할지 여부를 결정한다.

```c
void doIOOperation(Process selectedProcess, priority_queue *readyQueue, priority_queue *waitingQueue, scheduling_method sch_method);
```

doIOOperation 구현부의 메인 로직은 다음과 같다

```c
1. isIOBusy 상태를 true로 바꾼다.
    
2. 들어온 프로세스의 io_burst_time 을 1만큼 깎는다.
    
3. 다음에도 해당 프로세스가 I/O operation을 마저 수행할지, 끝내고 readyQueue로 갈지 결정한다.
    
4. I/O busrt time 남아있는데 cpu burst time 1 이하가 아니면서, I/O burst time이 끝났거나, 1/2 확률로 I/O 수행을 끝내고자 할 때, 현재 선택된 프로세스를 ready Queue에 다시 넣어주고 isIOBusy는 false로 바꾼다.

5. 만약 I/O busrt time 남아있는데 cpu burst time 1 이하라면 무조건 다음 I/O 수행 프로세스도 해당 프로세스가 되어야한다. 왜냐하면 I/O 수행하고 ISR 처리하기 위한 최소 cpu burst time(1)이 남아있어야하기 때문이다.
```



특정 프로세스가 CPU를 할당받아 수행하게 되었을때 실행되는 함수이다. 들어온 process의 cpu_burst_time, continued_time 등을 변경하고 다음에도 해당 프로세스가 CPU operation을 수행할지 여부를 결정한다. 프로세스의 수행이 끝났을 때의 처리도 한다.

```c
void doCPUOperation(Process selectedProcess, priority_queue *waitingQueue, priority_queue *terminatedQueue);
```

doCPUOperation 구현부의 메인 로직은 다음과 같다

```c
1. isCpuBusy 상태를 true로 바꾼다.

2. 들어온 프로세스의 cpu_burst_time 을 1만큼 깎는다.

3. 들어온 프로세스의 continued_time 을 1만큼 늘린다.

4. 만약 cpu_burst_time 이 0 이 된다면 해당 프로세스를 terminatedQueue에 넣고 isCpuBusy 상태를 false 로 바꾼다.

5. 다음에도 해당 프로세스가 CPU operation을 마저 수행할지, I/O 발생을 위해 waiting Queue에 들어갈지 결정한다.

6. 만약 I/O burst time이 남아있으면서 1/2 확률로 waiting Queue에 들어가고자 할때, 또는 cpu burst time이 1 이하로 남아있을 때 해당 프로세스를 waiting Queue로 넣는다. 이때 해당 프로세스의 continued_time은 0으로 초기화 하고 isCpuBusy 상태는 false 로 바꾼다.

7. 만약 I/O burst time이 남아있는데 cpu burst time이 1 이하로 남아있다면 무조건 waitingQueue로 들어와서 해당 I/O 작업을 다 끝내고 돌아온 후에 남은 cpu 작업을 수행해야 한다. CPU burst time이 0이 됐는데 I/O burst time이 남아있는 경우가 존재하면 안되기 때문이다.
```



## Ⅲ 결론

### 1. 알고리즘의 비교 분석

   각 프로세스 간의 성능 분석을 위해 Average Waiting Time과 Average Turnaround Time을 계산 할 때, 좀 더 정확한 평균 값을 내기 위해 각 스케쥴링 마다 10번의 evaluation 작업을 수행하였다. 따라서 최종  Average Waiting Time은 각 evaluation에서 측정된 Average Waiting Time을 더한 후 10으로 나눈 값이고 Average Turnaround Time 도 마찬가지이다.

   최종적으로 연산된 각 프로세스 간의 평균 Waiting Time과 Turnaround Time을 비교하면 다음과 같다.  

![evaluate](https://github.com/sujinnaljin/CPU_Scheduler/tree/master/images/evaluate.png)



















