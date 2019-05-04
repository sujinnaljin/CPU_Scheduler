//
//  main.c
//  CPU_Scheduler
//
//  Created by 강수진 on 01/05/2019.
//  Copyright © 2019 강수진. All rights reserved.
//

/*
 CPU Scheduling Simulator 의 구성 (예시 함수)
 • Create_Process(): 실행할 프로세스를 생성하고 각 프로세스에 데이터가 주어진다. (Random data 부여)
    o Process ID
    o CPU burst time
    o I/O burst time
    o Arrival time
    o Priority
 • Implementation of I/O operation (Random I/O 발생 여부 & I/O 횟수)
 • Config(): 시스템 환경 설정
    o Ready Queue / Waiting Queue
 • Schedule(): CPU 스케줄링 알고리즘을 구현한다.
    o FCFS(First Come First Served) //먼저 오는거 먼저 수행. IO 발생하면 다시 뒤로 가나?? . 우선순위가 arrival time 으로 하면 됨. io 발생하면 arrival time 다시 설정 해서 push 해주고
    o Non Preemptive - SJF(Shortest Job First) //적게 남아있는거 수행. cpu burst time 적게 남아있는게우선 순위 높다고 설정해주면 됨
    o Non Preemptive - Priority
    o RR(Round Robin) //먼저 들어온거되 타임 퀀텀 주면 됨
    o Preemptive – SJF
    o Preemptive – Priority
 • Evaluation(): 각 스케줄링 알고리즘들간 비교 평가한다.
    o Average waiting time
    o Average turnaround time
 • Gantt Chart
 */
#include <stdio.h>
#include <stdlib.h> //역할 : rand()
#include <time.h> //역할 : time(null)
#include <unistd.h> // sleep()
#define MAX_SIZE 3 //최대 10개의 프로세스 생성

typedef enum {false, true} bool;

//Process 관련
enum Priority {user_interactive = 0, user_initiated = 1, utility = 2, background = 3};

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
    enum Priority priority;
} Process;

//Priority Queue 관련
typedef struct priority_queue {
    Process heap[MAX_SIZE];
    int size;
}priority_queue;

void queueConfig(priority_queue* readyQ, priority_queue* waitingQ);
void swap(Process *a, Process *b);
int pq_push(priority_queue *q, Process value, scheduling_method scheduling_method);
Process pq_pop(priority_queue *q, scheduling_method scheduling_method);
int isEmpty(priority_queue *q);

Process createRandomProcess(void);

//전역변수
int unique_process_id;
bool isCpuBusy;
bool isIOBusy;
int currentTime = 0;
Process initialProcessArr[10];
Process runningCPUProcess;
Process runningIOProccess;

void doIOOperation(Process selectedProcess, priority_queue *readyQueue, priority_queue *waitingQueue, scheduling_method sch_method){

    printf("IO 수행 프로세스 : %d\n", selectedProcess.process_id);
    isIOBusy = true;
    
    //들어왔다는건 일단 수행되었다는 거니까 io burst time 깎아줌.
    selectedProcess.io_burst_time--;
    
    //io burst time 끝나면 종료
    if(selectedProcess.io_burst_time == 0) {
        if(selectedProcess.cpu_burst_time > 0){
            int tempCurrent = currentTime+1;
            selectedProcess.arrival_time = tempCurrent;
            printf("ready Queue에 #%d 들어감\n", selectedProcess.process_id);
            pq_push(readyQueue, selectedProcess, sch_method);
        }
        isIOBusy = false;
        return;
    }
    
    //만약 io busrt time 남아있는데 cpu burst time 1 이하이면 무조건 다음 io 도 해당 프로세스로 되어야함.
    //왜냐하면 io 수행하고 isr 처리하기 위한 cpu burst time 남아있어야하기 때문
    if(selectedProcess.io_burst_time > 0 && selectedProcess.cpu_burst_time <= 1){
        runningIOProccess = selectedProcess;
        return;
    }
    
    //IO 다음에도 수행할지 결정. 1/2 확률 && io burst time이 남아있으면 수행
    if(rand() % 2 == 0 && selectedProcess.io_burst_time > 0){
        //io 끝내겠다 -> readyQueue 에 넣기. 스케쥴링 따라서 넣어줘야 함.
        int tempCurrent = currentTime+1;
        selectedProcess.arrival_time = tempCurrent;
         printf("ready Queue에 #%d 들어감\n", selectedProcess.process_id);
        pq_push(readyQueue, selectedProcess, sch_method);
        isIOBusy = false;
    } else {
        //IO operation 다음에도 얘로 하겠다.
        runningIOProccess = selectedProcess;
    }
}

void doCPUOperation(Process selectedProcess, priority_queue *waitingQueue, priority_queue *terminatedQueue){
    isCpuBusy = true;
    printf("CPU 수행 프로세스 : %d\n", selectedProcess.process_id);
    
    //들어왔다는건 일단 수행되었다는 거니까 cpu burst time 깎아줌.
    selectedProcess.cpu_burst_time--;
    
    //cpu burst time 끝나면 종료
    if(selectedProcess.cpu_burst_time == 0) {
        //끝난 프로세스의 처음 상태 담아줌. selectedProcess 는 runningCPUProcess가 넘어와서 cpu_burst_time등이 변경되어있는 상태.
        int tempCurrent = currentTime+1;
        initialProcessArr[selectedProcess.process_id].terminate_time = tempCurrent;
         printf("terminated Queue에 #%d 들어감\n", selectedProcess.process_id);
        pq_push(terminatedQueue, initialProcessArr[selectedProcess.process_id], FCFS_scheduling);
        isCpuBusy = false;
        return;
    }
    
    //IO 수행여부 결정. 1/2 확률 && io burst time이 남아있으면 수행
    if(rand() % 2 == 0 && selectedProcess.io_burst_time > 0){
        //cpu 끝내겠다 -> IO 에 넣기. 온 순서대로 넣어줘야 함.
        int tempCurrent = currentTime+1;
        selectedProcess.arrival_time = tempCurrent;
         printf("waiting Queue에 #%d 들어감\n", selectedProcess.process_id);
        pq_push(waitingQueue, selectedProcess, FCFS_scheduling);
        isCpuBusy = false;
    }
    
    else if (selectedProcess.io_burst_time > 0 && selectedProcess.cpu_burst_time <= 1) {
        int tempCurrent = currentTime+1;
        selectedProcess.arrival_time = tempCurrent;
         printf("waiting Queue에 #%d 들어감\n", selectedProcess.process_id);
        pq_push(waitingQueue, selectedProcess, FCFS_scheduling); //근데 이때 waiting 큐에 들어가면 바로 시작해서 안돼
        isCpuBusy = false;
    }
    
    else {
        //IO operation 발생 안하는 경우. 다음 수행될 프로세스도 이거란 얘기.
        runningCPUProcess = selectedProcess;
    }
}


//1. readyQueue에서 프로세스 한개 꺼내옴. -> pop
//2. while(총 남은 cpu burst time > 0) 까지 랜덤 타임 동안 수행. 수행할 동안 cpu burst time 깎아줌.
//2-1. 중간에 I/O operation 발생 위해 웨이팅 큐로 보냄. (이때 io burst time 이 남아있을때 오퍼레이션 발생하게 하기)
//3. 웨이팅 큐로 간 후에 I/O operation 수행하는데 while(총 남은 io burst time > 0) 까지 랜덤 타임 동안 수행. 수행 동안 io burst time 깎아줌.
//3-1. io busrt time == 0 돼서 완전히 끝나거나, 중간에 수행하다 끝나면 readyQueue로 보냄. (이때 cpu burst time 이 0 이상이어야 함.)
//3-2. cpu burst time 0 되기 전에 io operation 끝나게 처리해야함. 왜냐면 io operation 끝나면 interrupt 보낸 후에 cpu가 isr 처리하니까 cpu burst time 이 0 됐는데 io operation 이 남아있는 경우 없을 것. 최소 단위 1초로 처리할 것이니까, 만약 cpu burst time <=1 이면 이때는 io burst time 다 쓰게하기
void FCFS(scheduling_method sch_method, priority_queue *jobQueue, priority_queue *readyQueue, priority_queue *waitingQueue, priority_queue *terminatedQueue){
    //FCFS는 FirstCome FisrtServed.
    //tie break 상황. arrival time이 같을때? 그냥 queue에서 맨 상위 노드에 있는걸로. 그건 먼저 들어온거겟지.
    while (terminatedQueue-> size != MAX_SIZE) {
        
        printf("현재 시간은 : %d 초\n", currentTime);
        
        //해당 시간에 job 큐에서 reday 큐로 할당.
        while(true){
            if (jobQueue->size > 0) {
                Process tempPop = pq_pop(jobQueue, FCFS_scheduling);
                if(tempPop.arrival_time == currentTime){
                     printf("ready Queue에 #%d 들어감\n", tempPop.process_id);
                    pq_push(readyQueue, tempPop, sch_method);
                } else {
                    pq_push(jobQueue, tempPop, FCFS_scheduling);
                    break;
                }
            } else {
                break;
            }
        }
        
        //2. cpu busy 상태 체크
        if (isCpuBusy) {
            doCPUOperation(runningCPUProcess, waitingQueue, terminatedQueue);
        } else {
            //cpu idle 상태면 readyQueue에서 꺼내서 cpu 할당해줘야함. 그 전에 readyQueue 비어있는 상태인지 체크. 비어있다면 cpu idle 상태.
            if(readyQueue->size > 0){
                //cpu 작업 고!. 초기상태일수도 있지 이게.
                Process selectedProcess = pq_pop(readyQueue, sch_method);
                runningCPUProcess = selectedProcess;
                doCPUOperation(runningCPUProcess, waitingQueue, terminatedQueue);
            } else {
                //idle
                printf("CPU idle\n");
            }
        }
       
        
        if (isIOBusy) {
            //하던 작업 마저 수행
            doIOOperation(runningIOProccess, readyQueue, waitingQueue, sch_method);
        } else {
            //io idle 상태면 waitingQueue에서 꺼내서 io 할당해줘야함. 그 전에 waitingQueue 비어있는 상태인지 체크. 비어있다면 io idle 상태.
            if(waitingQueue->size > 0){
                //IO 작업 고! arrival time 대로 진행되어야함.
                Process selectedProcess = pq_pop(waitingQueue, FCFS_scheduling);
                int tempCurrent = currentTime+1;
                if(selectedProcess.arrival_time == tempCurrent) {
                    //만약 바로 들어온거면 스루
                    pq_push(waitingQueue, selectedProcess, FCFS_scheduling);
                    printf("IO idle\n");
                } else {
                    runningIOProccess = selectedProcess;
                    doIOOperation(runningIOProccess, readyQueue, waitingQueue, sch_method);
                }
            } else {
                //io idle
                printf("IO idle\n");
                
            }
        }
        currentTime++;
       
    } //while
    
    while (terminatedQueue -> size != 0) {
        Process process = pq_pop(terminatedQueue, FCFS_scheduling);
        printf("process # %d's arrival time : %d, cpu burst time : %d, io burst : %d, terminated : %d\n", process.process_id, process.arrival_time, process.cpu_burst_time, process.io_burst_time, process.terminate_time);
    }

    
    //1-1. 생성한 프로세스 readyQueue에 할당
    //2. 레디큐에 있는 root process(pop) 빼서 cpu 할당.
    //2-1. while(총 남은 cpu burst time > 0) 까지 랜덤 타임 동안 수행. 수행할 동안 cpu burst time 깎아줌. 그리고 이때 cpu burst time 이 1초 이하일때 io 수행 남아있으면 가서 다 수행시켜야함.
    //3. cpu 수행하다가 io burst time 남은게 있으면 랜덤한 시간에 I/O operation 발생. 그리고 웨이팅 큐 할당
    //3-1-1. 웨이팅 큐로 갈때 readyQueue에서는 다음 프로세스 빼서 cpu 할당 반복.
    //3-1-2. 만약 다 readyQueue에서 수행할 프로세스 없으면 idle 상태. (다 수행해서 없는거랑 구분해야함)
    //3-2-1. 웨이팅 큐는 FIFO 구조. 일반 큐로 구현.
    //3-2-2. 프로세스 하나 빼내서 수행해야할 시간 정해줘야함. 이때 cpu burst 남은 시간이 1 이하라면, io 수행 후 cpu에게 isr 처리할 시간 남겨줘야 하므로, 이번에 io 수행해야할 시간 = io 남은 시간 else I/O burst time 보다 같거나 작은 범위로 랜덤 숫자 할당하고 그 시간동안 i/o burst 수행.
    //3-2-3. 할당된 burst time이 끝나면 다시 레디 큐 할당. 이때 arrival time 다시 설정해서 넣어줘야함.
    //4. 프로세스 모두 수행했다면(ready, waiting is Empty && unique_id == 9) 종료
    
    /*   for (int i = 0; i < 10; i++)
     {
     int value;
     scanf("%d", &value);
     Process randomProccess = createRandomProcess();
     initialProcessArr[randomProccess.process_id] = randomProccess; //초기 값 복사. 나중에 값 계산 위해.
     pq_push(readyQueue, randomProccess, sch_method);
     }
     
     
     while (!isEmpty(readyQueue)) {
     Process popped = pq_pop(readyQueue, sch_method);
     printf("arrival time is %d, process id is %d \n", popped.arrival_time, popped.process_id);
     }*/
} //FCFS

int main(int argc, const char * argv[]) {

    //queueConfig(&readyQueue, &waitingQueue);
    priority_queue readyQueue = {.size = 0};
    priority_queue waitingQueue = {.size = 0};
    priority_queue terminatedQueue = {.size = 0};
    priority_queue jobQueue = {.size = 0};

    unique_process_id = 0;
    //1. jobQueue에 10개를 랜덤으로 넣어준다
    for (int i = 0; i<MAX_SIZE; i++) {
        Process randomProcess = createRandomProcess();
        printf("created process is # %d, arrival time : %d, cpu burst : %d, io burst : %d \n",randomProcess.process_id, randomProcess.arrival_time, randomProcess.cpu_burst_time, randomProcess.io_burst_time);
        pq_push(&jobQueue, randomProcess, FCFS_scheduling);
        initialProcessArr[randomProcess.process_id] = randomProcess; //나중에 값 계산하기 위함
    }
    
   
    isCpuBusy = false;
    isIOBusy = false;

    /*char *guide_letter =
    "수행하고자하는 알고리즘의 번호를 선택해주세요\n"
    "0 : FCFS\n"
    "1 : SJF\n"
    "2 : STRF"
    "3 : Preemtive Priority\n"
    "4 : Non Preempvie Priority\n"
    "5 : RR\n"
    ;
    printf("%s",guide_letter);

    int sch_method;
    scanf("%d", &sch_method);*/
    int sch_method = 0;
    switch (sch_method) {
            case FCFS_scheduling:
            FCFS(sch_method, &jobQueue, &readyQueue, &waitingQueue, &terminatedQueue);
            break;
            case SJF_scheduling:
            printf("SJF 선택\n");
            break;
            case STRF_scheduling:
            printf("STRF 선택\n");
            break;
            case PreemtivePriority_scheduling:
            printf("preemtive 선택\n");
            break;
            case NonPreempviePriority_scheduling:
            printf("non preemtive priority 선택\n");
            break;
            case RR_scheduling:
            printf("RR 선택\n");
            break;
        default:
            printf("유효하지 않은 입력값입니다. 프로그램을 종료합니다. \n");
            return 0;
    }

    return 0;
}

//////////////Process//////////////

Process createRandomProcess() {
    int process_id = unique_process_id; //1부터 unique process ID 시작.
    int cpu_burst_time = rand()%3; // 0부터 9초까지
    //cpu 1초면 io 수행시간 안되므로 0으로
    int io_burst_time = cpu_burst_time <= 1 ? 0 : rand()%3;
    int arrival_time = rand()%3; // 0부터 9까지 랜덤 타임으로 arrive
    enum Priority priority = (rand() % 4);
    Process process = {.process_id = process_id, .cpu_burst_time = cpu_burst_time, .io_burst_time = io_burst_time, .arrival_time = arrival_time, .priority = priority};

    unique_process_id = unique_process_id+1; //전역변수. 프로세스 생성될때마다 1씩 증가
    return process;
}

//////////////Queue/////////////////

void queueConfig(priority_queue* readyQ, priority_queue* waitingQ){
    readyQ->size = 0;
    waitingQ->size = 0;
}

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
            case FCFS_scheduling :
            while (current > 0 && (q ->heap[current].arrival_time) <= (q ->heap[parent].arrival_time)) {
                swap(&(q->heap[current]), &(q ->heap[parent]));
                current = parent;
                parent = (parent - 1) / 2;
            }
            break;
            case SJF_scheduling:
            while (current > 0 && (q ->heap[current].cpu_burst_time) <= (q ->heap[parent].cpu_burst_time)) {
                swap(&(q->heap[current]), &(q ->heap[parent]));
                current = parent;
                parent = (parent - 1) / 2;
            }
            break;
            case STRF_scheduling:
            printf("STRF push\n");
            break;
            case PreemtivePriority_scheduling:
            printf("preemtive push\n");
            break;
            case NonPreempviePriority_scheduling:
            printf("non preemtive priority push\n");
            break;
            case RR_scheduling:
            printf("RR push\n");
            break;
        default: printf("not special"); break;
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
            case FCFS_scheduling :
            while (leftChild < q->size) {
                if ((q->heap[maxNode]).arrival_time >= (q->heap[leftChild]).arrival_time) {
                    maxNode = leftChild;
                }
                if (rightChild < q->size && q->heap[maxNode].arrival_time >= q->heap[rightChild].arrival_time) {
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
            case SJF_scheduling:
            while (leftChild < q->size) {
                if ((q->heap[maxNode]).cpu_burst_time >= (q->heap[leftChild]).cpu_burst_time) {
                    maxNode = leftChild;
                }
                if (rightChild < q->size && q->heap[maxNode].cpu_burst_time >= q->heap[rightChild].cpu_burst_time) {
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
            case STRF_scheduling:
            printf("STRF pop\n");
            break;
            case PreemtivePriority_scheduling:
            printf("preemtive pop\n");
            break;
            case NonPreempviePriority_scheduling:
            printf("non preemtive priority pop\n");
            break;
            case RR_scheduling:
            printf("RR pop\n");
            break;
        default: printf("not special"); break;


    }



    return ret;
}



int isEmpty(priority_queue *q) {
    if (q->size == 0) {
        return 1;
    }
    else {
        return 0;
    }
}
