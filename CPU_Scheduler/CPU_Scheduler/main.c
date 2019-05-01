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
    o FCFS(First Come First Served)
    o Non Preemptive - SJF(Shortest Job First)
    o Non Preemptive - Priority
    o RR(Round Robin)
    o Preemptive 방식 적용 – SJF, Priority
 • Evaluation(): 각 스케줄링 알고리즘들간 비교 평가한다.
    o Average waiting time
    o Average turnaround time
 • Gantt Chart
 */
#include <stdio.h>

int main(int argc, const char * argv[]) {
    // insert code here...
    printf("Hello, World!\n");
    return 0;
}
