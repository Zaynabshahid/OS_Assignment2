#include <stdio.h>
#include <stdlib.h>

typedef struct {
    int pid;
    int priority;
    int burst_time;
} Process;

void sortByPriority(Process *p, int n) {
    for(int i=0;i<n-1;i++){
        for(int j=i+1;j<n;j++){
            if(p[i].priority<p[j].priority){
                Process temp=p[i];
                p[i]=p[j];
                p[j]=temp;
            }
        }
    }
}

int main(){
    int n;
    printf("Enter number of processes: ");
    scanf("%d",&n);

    Process *p=(Process*)malloc(n*sizeof(Process));

    for(int i=0;i<n;i++){
        printf("Process %d ID: ", i+1);
        scanf("%d",&p[i].pid);
        printf("Process %d Priority: ", i+1);
        scanf("%d",&p[i].priority);
        printf("Process %d Burst Time: ", i+1);
        scanf("%d",&p[i].burst_time);
    }

    sortByPriority(p,n);

    printf("\nProcess Execution Order:\nPID\tPriority\tBurst Time\n");
    for(int i=0;i<n;i++){
        printf("%d\t%d\t\t%d\n",p[i].pid,p[i].priority,p[i].burst_time);
    }

    free(p);
    return 0;
}

