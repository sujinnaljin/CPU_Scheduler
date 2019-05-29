//
//  main.c
//  CPU_Scheduler
//
//  Created by 강수진 on 01/05/2019.
//  Copyright © 2019 강수진. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h> //rand()
#include <string.h> //strcat, sprintf
#include <time.h>

#define MAX_SIZE 5 //최대 생성 프로세스 사이즈 지정
#define TIME_QUANTUM 3
#define MAX_CPU_BURST_TIME 10
#define MAX_IO_BURST_TIME 5
#define MAX_PRIORITY 4
#define MAX_ARRIVAL_TIME 10

typedef enum {false, true} bool;

typedef enum {
    FCFS_scheduling = 0,
    SJF_scheduling = 1,
    STRF_scheduling = 2,
    PreemtivePriority_scheduling = 3,
    NonPreempviePriority_scheduling = 4,
    RR_scheduling = 5
} scheduling_method;


typedef struct {
    int process_id;
    int cpu_burst_time;
    int io_burst_time;
    int arrival_time;
    int terminate_time;
    int continued_time;
    int priority;
} Process;

//Priority Queue 관련
typedef struct priority_queue {
    Process heap[MAX_SIZE];
    int size;
}priority_queue;

void swap(Process *a, Process *b);
int pq_push(priority_queue *q, Process value, scheduling_method scheduling_method);
Process pq_pop(priority_queue *q, scheduling_method scheduling_method);
Process createRandomProcess(int uniqueId);

//전역변수
int currentTime = 0;
bool isCpuBusy = false;
bool isIOBusy = false;

Process initialProcessArr[MAX_SIZE]; //나중에 값 계산 위함
Process runningCPUProcess;
Process runningIOProccess;

char timeLine[65536];
char topLine[65536];
char bottomLine[65536];
char middleLineIO[65536];
char middleLineCPU[65536];


//메인 알고리즘 관련
void doScheduling(scheduling_method sch_method, priority_queue *jobQueue, priority_queue *readyQueue, priority_queue *waitingQueue, priority_queue *terminatedQueue);
void doIOOperation(Process selectedProcess, priority_queue *readyQueue, priority_queue *waitingQueue, scheduling_method sch_method);
void doCPUOperation(Process selectedProcess, priority_queue *waitingQueue, priority_queue *terminatedQueue);


//간트 차트 관련
void drawTimeLine(void);
void drawTopBottomLine(void);
void drawMiddleLine(bool isCpu, bool isIdle);
void printGanttChart(void);

//처음 프로세스 상태 출력 with table
void showInitialProcess(void);

//알고리즘 성능 체크 출력 with table
void evaluateAlgorithm(priority_queue *terminatedQueue);

//다른 알고리즘 수행 의사
bool isContinueOtherAlgo(void);


//초기환경 세팅
void initailizeQueue(priority_queue *queue);
void setEnv(priority_queue *jobQueue, priority_queue *readyQueue, priority_queue *waitingQueue, priority_queue *terminatedQueue);

int main(int argc, const char * argv[]) {
    
    srand((unsigned int)time(NULL));
    //Queue 생성
    priority_queue jobQueue = {.size = 0};
    priority_queue readyQueue = {.size = 0};
    priority_queue waitingQueue = {.size = 0};
    priority_queue terminatedQueue = {.size = 0};
    
  
    //초기에 프로세스 생성
    for (int i = 0; i<MAX_SIZE; i++) {
        initialProcessArr[i] = createRandomProcess(i); //나중에 값 계산하기 위함
    }
    
    //프로세스 상태 프린트하는 함수
    showInitialProcess();

    while (true) {
        //사용자 입력 받아서 해당 스케쥴러 알고리즘 실행
        char *guide_letter =
        "수행하고자하는 알고리즘의 번호를 선택해주세요. 종료를 원하시면 이외의 숫자를 눌러주세요\n"
        "0 : FCFS\n"
        "1 : SJF\n"
        "2 : STRF\n"
        "3 : Preemtive Priority\n"
        "4 : Non Preempvie Priority\n"
        "5 : RR\n"
        ;
        
        printf("%s",guide_letter);
        
        int sch_method;
        scanf("%d", &sch_method);
        
        if(0<= sch_method && sch_method <= 5){
            //큐 비워주고, 타임 0으로, CPU / IO 상태 false 로 등등..
            setEnv(&jobQueue, &readyQueue, &waitingQueue, &terminatedQueue);
            //스케쥴링 시작
            doScheduling(sch_method, &jobQueue, &readyQueue, &waitingQueue, &terminatedQueue);
            //모두 마친 후 스케쥴링 알고리즘 성능 평가
            evaluateAlgorithm(&terminatedQueue);
            //간트 차트 그림
            printGanttChart();
            
            if (!isContinueOtherAlgo()) {
                puts("\nbye~");
                break;
            }
        } else {
            puts("\nbye~");
            break;
        }
    }
    return 0;
}

//////////////Process//////////////

Process createRandomProcess(int uniqueId) {
    int process_id = uniqueId;
    int cpu_burst_time = rand()%(MAX_CPU_BURST_TIME)+1; // cpu burst time 이 1 이상이어야 하니까 1 더해줌. 1부터 max time 까지.
    int io_burst_time = cpu_burst_time <= 1 ? 0 : rand()%(MAX_IO_BURST_TIME+1); //cpu 시간 1 이하인데 io 가 있을 수는 없으니까 cpu busrt 1이면 io burst 0으로 할당
    int arrival_time = rand()%(MAX_ARRIVAL_TIME+1);
    int priority = rand() % (MAX_PRIORITY+1);
    Process process = {.process_id = process_id, .cpu_burst_time = cpu_burst_time, .io_burst_time = io_burst_time, .arrival_time = arrival_time, .priority = priority};
    return process;
}

//////////////Queue/////////////////

void swap(Process *a, Process *b) {
    Process tmp = *a;
    *a = *b;
    *b = tmp;
}

int pq_push(priority_queue* q, Process value, scheduling_method scheduling_method) {

    int size = q -> size;
    //큐에 모두 데이터가 찼으면 return 시킴
    if (size + 1 > MAX_SIZE) {
        return 0;
    }

    q -> heap[size] = value; //마지막 빈 자리에 value 할당

    //재정렬
    int current = size; //현재 위치한 인덱스
    int parent = (size - 1) / 2; //완전 이진트리에서 parent 인덱스

    switch(scheduling_method){
        case FCFS_scheduling : case RR_scheduling :
            while (current > 0 && (q ->heap[current].arrival_time) < (q ->heap[parent].arrival_time)) {
                swap(&(q->heap[current]), &(q ->heap[parent]));
                current = parent;
                parent = (parent - 1) / 2;
            }
            break;
        case SJF_scheduling: case STRF_scheduling :
            while (current > 0 && (q ->heap[current].cpu_burst_time) < (q ->heap[parent].cpu_burst_time)) {
                swap(&(q->heap[current]), &(q ->heap[parent]));
                current = parent;
                parent = (parent - 1) / 2;
            }
            break;
        case NonPreempviePriority_scheduling: case PreemtivePriority_scheduling :
            while (current > 0 && (q ->heap[current].priority) < (q ->heap[parent].priority)) {
                swap(&(q->heap[current]), &(q ->heap[parent]));
                current = parent;
                parent = (parent - 1) / 2;
            }
            break;
        default:
            break;
    }

    q ->size++;
    return 1;
}

Process pq_pop(priority_queue* q, scheduling_method scheduling_method) {
    //비어있으면 return
    Process process = {.process_id = -1};
    if (q->size <= 0) return process;

    //우선 순위 큐에서 pop 할 것은 가장 우선 순위가 높은 노드, 즉 루트
    Process ret = q->heap[0];
    q->size--;

    //재정렬
    q->heap[0] = q->heap[q->size]; //루트에 가장 낮은거 올림

    int current = 0;
    int leftChild = current * 2 + 1;
    int rightChild = current * 2 + 2;
    int maxNode = current;

    switch(scheduling_method){
        case FCFS_scheduling : case RR_scheduling :
            while (leftChild < q->size) {
                //left child 가 있는데 max node의 arrival time 이 더 큰 경우
                if ((q->heap[maxNode]).arrival_time > (q->heap[leftChild]).arrival_time) {
                    maxNode = leftChild;
                }
                //right child 까지 있는데 max node(방금전까지 leftChild의 값)의 arrival time 이 더 큰 경우
                if (rightChild < q->size && q->heap[maxNode].arrival_time > q->heap[rightChild].arrival_time) {
                    maxNode = rightChild;
                }

                if (maxNode == current) {
                    break;
                }
                else {
                    swap(&(q->heap[current]), &(q->heap[maxNode]));
                    current = maxNode;
                    leftChild = current * 2 + 1;
                    rightChild = current * 2 + 2;
                }
            }
            break;
        case SJF_scheduling: case STRF_scheduling :
            while (leftChild < q->size) {
                if ((q->heap[maxNode]).cpu_burst_time > (q->heap[leftChild]).cpu_burst_time) {
                    maxNode = leftChild;
                }
                if (rightChild < q->size && q->heap[maxNode].cpu_burst_time > q->heap[rightChild].cpu_burst_time) {
                    maxNode = rightChild;
                }
                
                if (maxNode == current) {
                    break;
                }
                else {
                    swap(&(q->heap[current]), &(q->heap[maxNode]));
                    current = maxNode;
                    leftChild = current * 2 + 1;
                    rightChild = current * 2 + 2;
                }
            }
            break;
        case NonPreempviePriority_scheduling: case PreemtivePriority_scheduling :
            while (leftChild < q->size) {
                if ((q->heap[maxNode]).priority > (q->heap[leftChild]).priority) {
                    maxNode = leftChild;
                }
                if (rightChild < q->size && q->heap[maxNode].priority > q->heap[rightChild].priority) {
                    maxNode = rightChild;
                }
                
                if (maxNode == current) {
                    break;
                }
                else {
                    swap(&(q->heap[current]), &(q->heap[maxNode]));
                    current = maxNode;
                    leftChild = current * 2 + 1;
                    rightChild = current * 2 + 2;
                }
            }
            break;
        default:
            break;
    }
    return ret;
}


////////////메인 알고리즘//////////////
void doScheduling(scheduling_method sch_method, priority_queue *jobQueue, priority_queue *readyQueue, priority_queue *waitingQueue, priority_queue *terminatedQueue){
    //tie break 상황은 그냥 queue에서 맨 상위 노드에 있는걸로.
    while (terminatedQueue-> size != MAX_SIZE) {
        drawTimeLine();
        drawTopBottomLine();
        
        //생성된 프로세스 arrivalTime 체크해서 job 큐에서 reday 큐로 할당. 원래는 랜덤으로 프로세스 들어오겠지만 각 스케쥴링마다 같은 프로세스 사용해야하므로 jobQueue라는 저장공간에 있는 프로세스 꺼내다 씀
        while(true){
            if (jobQueue->size > 0) {
                //arrivalTime 순으로 할당 된거 꺼냄. FCFS 방식
                Process tempPop = pq_pop(jobQueue, FCFS_scheduling);
                if(tempPop.arrival_time == currentTime){
                    //현재 시간에 arrive 됐으면 readyQueue에 할당.
                    pq_push(readyQueue, tempPop, sch_method);
                } else {
                    //현재 시간 아니면 다시 집어넣고 while 종료
                    pq_push(jobQueue, tempPop, FCFS_scheduling);
                    break;
                }
            } else {
                break;
            }
        }

    
        if (isCpuBusy) {
            //non preemtive 는 하던 작업 마저 수행 - cpu가 busy 상태면 전에 수행하던 프로세스가 전역변수 runningCPUProcess 에 할당되어 있을 것
            //preemtive 는 ready queue에 다음 프로세스가 있을때 현재 프로세스보다 더 프라이어리티 높나 체크. 현재가 높으면 하던거 수행. 안높으면 원래 하던거(running CPU process)는 레디 큐로 할당해주고 우선순위 높은거를 running cpu process로 바꿔야함
            //밑에 공통적으로 doCPUOperation() 있음. switch 문에서는 runningProcess 뭘로 할거냐 결정해주는 것
            switch (sch_method) {
                case FCFS_scheduling : case SJF_scheduling : case NonPreempviePriority_scheduling :
                    break;
                case PreemtivePriority_scheduling :
                    if(readyQueue -> size > 0){
                        Process tempProcess = pq_pop(readyQueue, sch_method);
                        if(runningCPUProcess.priority > tempProcess.priority) {
                            //현재 진행 프로세스 우선순위가 더 낮은 상황(priority 값이 낮은게 우선순위 높은 것)
                            pq_push(readyQueue, runningCPUProcess, sch_method); //원래 프로세스 레디큐에 집어넣고
                            runningCPUProcess = tempProcess;  //runningProcess를 꺼낸걸로 바꿔준다음에 수행
                        } else {
                            //현재 진행 프로세스 우선순위가 같거나 높은 상황
                            pq_push(readyQueue, tempProcess, sch_method); //비교 위해 꺼낸 것 다시 넣어줌
                        }
                    }
                    break;
                case STRF_scheduling :
                    if(readyQueue -> size > 0){
                        Process tempProcess = pq_pop(readyQueue, sch_method);
                        if(runningCPUProcess.cpu_burst_time > tempProcess.cpu_burst_time) {
                            pq_push(readyQueue, runningCPUProcess, sch_method);
                            runningCPUProcess = tempProcess;
                        } else {
                             pq_push(readyQueue, tempProcess, sch_method);
                        }
                    }
                    break;
                case RR_scheduling:
                    //만약에 레디큐에 프로세스 없으면 원래 수행되던 애가 타임 퀀텀 관계 없이 계속 수행될 것.
                    //레디큐에 프로세스들 있으면 현재 돌리고 있는 프로세스의 타임퀀텀 체크
                    if(readyQueue -> size > 0){
                        Process tempProcess = pq_pop(readyQueue, sch_method);
                        //RR - 현재 진행되는 프로세스가 timequantum 이상으로 수행됐나 체크. 안넘었으면 이전 프로세스 계속 수행.
                        //넘었으면 레디큐에 있던 다른 프로세스 수행
                        if(runningCPUProcess.continued_time >= TIME_QUANTUM){
                            runningCPUProcess.continued_time = 0; //rr 타임 퀀텀 다다르면 원래 프로세스 지속 시간 초기화
                            //이번 타임(currentTime)에 마저 cpu 에 할당해서 수행시킬 것이 아니라, readyQueue 에 넣어줘야함
                            //이때 시간 다시 설정해서 레디큐에 들어가게 해야함. 안그러면 얘네들 초기값으로 들어가서 우선적으로 다시 수행될 것
                            runningCPUProcess.arrival_time = currentTime;
                            pq_push(readyQueue, runningCPUProcess, sch_method);
                            runningCPUProcess = tempProcess;
                        } else {
                            pq_push(readyQueue, tempProcess, sch_method);
                        }
                    }
                    break;
                default:
                    break;
            }// switch
            
            doCPUOperation(runningCPUProcess, waitingQueue, terminatedQueue); //위의 코드에 의해 결정된 runningCPUProcess 시행
            drawMiddleLine(true, false); //수행 될 process 그리기
        } else {
            //cpu not busy 상태면 readyQueue에서 꺼내서 cpu 할당해줘야함. 그 전에 readyQueue 비어있는 상태인지 체크. 비어있다면 cpu idle 상태.
            if(readyQueue->size > 0){
                Process selectedProcess = pq_pop(readyQueue, sch_method);
                runningCPUProcess = selectedProcess;
                doCPUOperation(runningCPUProcess, waitingQueue, terminatedQueue);
                drawMiddleLine(true, false);
            } else {
                //cpu idle
                drawMiddleLine(true, true);
            }
        }
        
        if (isIOBusy) {
            //io busy 하면 하던 작업 마저 수행
            doIOOperation(runningIOProccess, readyQueue, waitingQueue, sch_method);
            drawMiddleLine(false, false);
        } else {
            //io not busy 상태면 waitingQueue에서 꺼내서 io 할당해줘야함. 그 전에 waitingQueue 비어있는 상태인지 체크. 비어있다면 io idle 상태.
            if(waitingQueue->size > 0){
                //IO 작업 시작.  arrival time 대로 진행되어야하므로 fcfs 스케쥴링.
                Process selectedProcess = pq_pop(waitingQueue, FCFS_scheduling);
                int tempCurrent = currentTime+1;
            
                if(selectedProcess.arrival_time == tempCurrent) {
                    //만약 위의 cpu 작업에서 바로 들어온거라면 넘어감. 다시 waitingQueue에 그대로 넣어주고 스루
                    pq_push(waitingQueue, selectedProcess, FCFS_scheduling);
                    drawMiddleLine(false, true);
                } else {
                    runningIOProccess = selectedProcess;
                    drawMiddleLine(false, false);
                    doIOOperation(runningIOProccess, readyQueue, waitingQueue, sch_method);
                }
            } else {
                //io idle
                drawMiddleLine(false, true);
            }
        }
        
        //cpu랑 io 체크 작업 다 했으니까 time 올려주고 다음 작업 수행 반복
        currentTime++;
    } //while
    drawTimeLine(); //마지막 시간 한번 더 그려줘야함
} //doScheduling

void doIOOperation(Process selectedProcess, priority_queue *readyQueue, priority_queue *waitingQueue, scheduling_method sch_method){
    
    isIOBusy = true;
    
    //들어왔다는건 일단 수행되었다는 거니까 io burst time 깎아줌.
    selectedProcess.io_burst_time--;
    
    //(io burst time 끝날때 (isr 처리 위한 cpu burst time 무조건 남아있을 것) || 1/2 확률로 IO 수행 끝낼 때) && (io busrt time 남아있는데 cpu burst time 1 이하가 아닐때)
    //만약 io busrt time 남아있는데 cpu burst time 1 이하이면 무조건 다음 io 도 해당 프로세스로 되어야함. 왜냐하면 io 수행하고 isr 처리하기 위한 cpu burst time 남아있어야하기 때문
    if((selectedProcess.io_burst_time == 0 || rand() % 2 == 0) && !(selectedProcess.io_burst_time > 0 && selectedProcess.cpu_burst_time <= 1)){
        //IO operation -> readyQueue 에 넣기. 스케쥴링 따라서 넣어줘야 함.
        int tempCurrent = currentTime+1;
        selectedProcess.arrival_time = tempCurrent;
        pq_push(readyQueue, selectedProcess, sch_method);
        isIOBusy = false;
    } else {
        //IO operation 다음에도 얘로 하겠다.
        runningIOProccess = selectedProcess;
    }
}

void doCPUOperation(Process selectedProcess, priority_queue *waitingQueue, priority_queue *terminatedQueue){
    isCpuBusy = true;
    
    //들어왔다는건 일단 수행되었다는 거니까 cpu burst time 깎아줌.
    selectedProcess.cpu_burst_time--;
    selectedProcess.continued_time++; //지속 시간 증가. for RR
    
    //cpu burst time 끝나면 종료
    if(selectedProcess.cpu_burst_time == 0) {
        //끝난 프로세스의 처음 상태에 terminate time만 추가해서 종료 큐에 담아줌.
        //selectedProcess 는 runningCPUProcess가 넘어와서 cpu_burst_time등이 변경되어있는 상태니까 초기의 상태로 evaluation 해야 함
        int tempCurrent = currentTime+1;
        initialProcessArr[selectedProcess.process_id].terminate_time = tempCurrent;
        pq_push(terminatedQueue, initialProcessArr[selectedProcess.process_id], FCFS_scheduling);
        isCpuBusy = false;
        return;
    }
    
    //IO 수행여부 결정. 1/2 확률 && io burst time이 남아있으면 수행 || io busrt 가 남아있는데 cpu busrt 1 이하로 남아있으면 waiting Queue로 넘어가서 수행 끝내고 와야함
    if((rand() % 2 == 0 && selectedProcess.io_burst_time > 0) || (selectedProcess.io_burst_time > 0 && selectedProcess.cpu_burst_time <= 1)){
        //cpu 끝내겠다 -> IO 에 넣기.
        int tempCurrent = currentTime+1;
        selectedProcess.arrival_time = tempCurrent; //현재 시간에는 cpu가 수행 이미 완료 됐고, 그 다음 시간에 waitingQueue로 들어갈 애니까 +1 해서 arrvalTime에 할당
        selectedProcess.continued_time = 0; //cpu 연속된 시간 0으로 다시 초기화
        pq_push(waitingQueue, selectedProcess, FCFS_scheduling);
        isCpuBusy = false;
    }
    else {
        //IO operation 발생 안하는 경우. 다음 수행될 프로세스도 이거란 얘기.
        runningCPUProcess = selectedProcess;
    }
}

//////////////간트 차트 관련///////////////
void drawTimeLine(){
    char time[10];
    sprintf(time, "%d   ", currentTime%10); //간트차트 예쁘게 그려지기 위해서 %10 해줘야함
    strcat(timeLine, time); //추가
}

void drawTopBottomLine(){
    strcat(topLine, " -- ");
    strcat(bottomLine, " -- ");
}

void drawMiddleLine(bool isCpu, bool isIdle){
    char* selectedLine = isCpu ? middleLineCPU : middleLineIO;
    if (isIdle){
        strcat(selectedLine, "|//|");
    } else {
        int pid = isCpu ? runningCPUProcess.process_id : runningIOProccess.process_id;
        char str[10];
        sprintf(str, "|P%d|", pid);
        strcat(selectedLine, str);
    }
}

void printGanttChart(){
    puts("<CPU Gantt Chart>");
    puts(topLine);
    puts(middleLineCPU);
    puts(bottomLine);
    puts(timeLine);
    puts("");
    puts("<IO Gantt Chart>");
    puts(topLine);
    puts(middleLineIO);
    puts(bottomLine);
    puts(timeLine);
}

/////////////처음 프로세스 상태////////////////
void showInitialProcess()
{
    
    puts("+-----+--------------+----------+----------------+---------------+");
    puts("| PID | Arrival Time | Priority | CPU Busrt Time | IO Burst Time |");
    puts("+-----+--------------+----------+----------------+---------------+");
    
    for(int i=0; i<MAX_SIZE; i++) {
        printf("| %2d  |      %2d      |    %2d    |       %2d       |       %2d      |\n"
               , initialProcessArr[i].process_id, initialProcessArr[i].arrival_time, initialProcessArr[i].priority, initialProcessArr[i].cpu_burst_time, initialProcessArr[i].io_burst_time);
        puts("+-----+--------------+----------+----------------+---------------+");
    }
    
}

////////////////알고리즘 성능 체크////////////
void evaluateAlgorithm(priority_queue *terminatedQueue){
    int totalWaitingTime = 0;
    int totalTurnaroundTime = 0;
    puts("+-----+--------------+-----------------+--------------+-----------------+----------------+---------------+----------+");
    puts("| PID | Waiting Time | Turnaround Time | Arrival Time | Terminated Time | CPU Busrt Time | IO Burst Time | Priority |");
    puts("+-----+--------------+-----------------+--------------+-----------------+----------------+---------------+----------+");
    while (terminatedQueue -> size != 0) {
        Process process = pq_pop(terminatedQueue, FCFS_scheduling);
        int waitingTime = process.terminate_time - process.arrival_time - process.cpu_burst_time;
        int turnaroundTime = process.terminate_time - process.arrival_time;
        totalWaitingTime += waitingTime;
        totalTurnaroundTime += turnaroundTime;
        printf("| %2d  |      %2d      |        %2d       |      %2d      |        %2d       |       %2d       |       %2d      |    %2d    |\n"
               , process.process_id, waitingTime, turnaroundTime, process.arrival_time, process.terminate_time, process.cpu_burst_time, process.io_burst_time, process.priority);
        puts("+-----+--------------+-----------------+--------------+-----------------+----------------+---------------+----------+");
    }
    printf("Average Waiting time : %d\n", totalWaitingTime/MAX_SIZE);
    printf("Average Turnaround time : %d\n\n", totalTurnaroundTime/MAX_SIZE);
}


////////////////초기환경 세팅////////////////
void initailizeQueue(priority_queue *queue){
    while (queue -> size > 0) {
        pq_pop(queue, FCFS_scheduling);
    }
}


void setEnv(priority_queue *jobQueue, priority_queue *readyQueue, priority_queue *waitingQueue, priority_queue *terminatedQueue){
    currentTime = 0;
    isCpuBusy = false;
    isIOBusy = false;
    initailizeQueue(jobQueue);
    initailizeQueue(readyQueue);
    initailizeQueue(waitingQueue);
    initailizeQueue(terminatedQueue);
    for (int i = 0; i < MAX_SIZE; i++) {
        pq_push(jobQueue, initialProcessArr[i], FCFS_scheduling); //나중에 time 체크하면서 빼낼때 arrival time 대로 빼내야 하니까 FCFS 스케쥴링대로
    }
    timeLine[0] = '\0';
    topLine[0] = '\0';
    bottomLine[0] = '\0';
    middleLineIO[0] = '\0';
    middleLineCPU[0] = '\0';
}

///////////다른 알고리즘 수행 의사///////////
bool isContinueOtherAlgo(){
    puts("\n 다른 알고리즘을 사용하시겠습니까? 계속 하시려면 1을, 종료를 원하시면 1 외의 숫자를 눌러주세요");
    int continueOtherAlgo;
    scanf("%d", &continueOtherAlgo);
    return continueOtherAlgo == 1 ? true : false;
}
