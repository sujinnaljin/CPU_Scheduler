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

#define MAX_SIZE 10 //최대 10개의 프로세스 생성

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
    enum Priority priority;
} Process;

//Queue 관련
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
time_t start_time;
int unique_process_id;
Process initialProcessArr[10];

void doIOOperation(){
    
}

//1. readyQueue에서 프로세스 한개 꺼내옴. -> pop
//2. while(총 남은 cpu burst time > 0) 까지 랜덤 타임 동안 수행. 수행할 동안 cpu burst time 깎아줌.
//2-1. 중간에 I/O operation 발생 위해 웨이팅 큐로 보냄. (이때 io burst time 이 남아있을때 오퍼레이션 발생하게 하기)
//3. 웨이팅 큐로 간 후에 I/O operation 수행하는데 while(총 남은 io burst time > 0) 까지 랜덤 타임 동안 수행. 수행 동안 io burst time 깎아줌.
//3-1. io busrt time == 0 돼서 완전히 끝나거나, 중간에 수행하다 끝나면 readyQueue로 보냄. (이때 cpu burst time 이 0 이상이어야 함.)
//3-2. cpu burst time 0 되기 전에 io operation 끝나게 처리해야함. 왜냐면 io operation 끝나면 interrupt 보낸 후에 cpu가 isr 처리하니까 cpu burst time 이 0 됐는데 io operation 이 남아있는 경우 없을 것. 최소 단위 1초로 처리할 것이니까, 만약 cpu burst time <=1 이면 이때는 io burst time 다 쓰게하기
void FCFS(scheduling_method sch_method, priority_queue *readyQueue, priority_queue *waitingQueue){
    //FCFS는 FirstCome FisrtServed.
    //tie break 상황. arrival time이 같을때? 그냥 queue에서 맨 상위 노드에 있는걸로. 그건 먼저 들어온거겟지.
    
    //1. 랜덤한 시간에(10초 내)로 한개씩 총 10개의 프로세스 생성
    
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
}

int main(int argc, const char * argv[]) {

    //queueConfig(&readyQueue, &waitingQueue);
    priority_queue readyQueue = {.size = 0};
    priority_queue waitingQueue = {.size = 0};
    unique_process_id = 0;
   
    start_time = time(NULL);
    char *guide_letter =
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
    scanf("%d", &sch_method);
    switch (sch_method) {
            case FCFS_scheduling:
            FCFS(sch_method, &readyQueue, &waitingQueue);
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
    int cpu_burst_time = rand()%10; //0부터 9초까지
    int io_burst_time = rand()%10;
    time_t current = time(NULL);
    int arrival_time = (int)(current-start_time);
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

    int current = size; //현재 위치한 인덱스
    int parent = (size - 1) / 2; //완전 이진트리에서 parent 인덱스

    //재정렬
    switch(scheduling_method){
            case FCFS_scheduling :
            while (current > 0 && (q ->heap[current].arrival_time) <= (q ->heap[parent].arrival_time)) {
                swap(&(q->heap[current]), &(q ->heap[parent]));
                current = parent;
                parent = (parent - 1) / 2;
            }
            break;
            case SJF_scheduling:
            printf("SJF push\n");
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
    
    //재정렬
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
            printf("SJF pop\n");
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
