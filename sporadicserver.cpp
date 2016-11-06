#include<bits/stdc++.h>
#include <pthread.h>
#include <semaphore.h>
using namespace std;
#include "allegro5/allegro.h"
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

#define MAX(a, b) ((a) > (b) ? ( a) : (b))
const float FPS = 60;
const int SCREEN_W = 640;
const int SCREEN_H = 480;
const int BOUNCER_SIZE = 32;
ALLEGRO_FONT *font;
int no_periodic,no_aperiodic;
int Ts,Cs,RA,RT,ds;

int maxTime;
struct aperiodic {
  int arrival;
  int C;

};
struct periodic {
  int C;
  int timeperiod;
};

struct jobsforQ
{
    int remainingC;
    int deadline;
    char type;
    int no;
};
struct replenishment
{
    int amount;
    int RT;

};

class CompareJobsQ
{
public:
    bool operator() (struct jobsforQ &lhs, struct jobsforQ &rhs)
    {
        if(lhs.deadline>rhs.deadline)
        return true;
        else
        return false;
    }
};
priority_queue<struct jobsforQ, vector<struct jobsforQ>, CompareJobsQ> readyQ;
queue<struct replenishment> replenishmentQ;

struct periodic * periodicTasks;
struct aperiodic * aperiodicJobs;

static int arrival_cmp(const void *a, const void *b)
{
  const struct aperiodic *da = (struct aperiodic *)a, *db = (struct aperiodic *)b;

  return da->arrival < db->arrival ? -1 : da->arrival > db->arrival;
}

static int timeperiod_cmp(const void *a, const void *b)
{
  const struct periodic *da = (struct periodic *)a, *db = (struct periodic *)b;

  return da->timeperiod < db->timeperiod ? -1 : da->timeperiod > db->timeperiod;
}

float U;
void calculateSchedule();
void drawSchedule();

int takeinput();

int schedubilityTest() ;

int main(int argc, char **argv)
{
    printf("Dynamic Sporadic Server");
    if(!takeinput()) {
      fprintf(stderr, "failed to takeinput!\n");
      return -1;
    }
int    schedulable = schedubilityTest();
if(schedulable){
cout<<"something\n";}
    calculateSchedule();
    drawSchedule();
}


void calculateSchedule(){

    qsort(periodicTasks, sizeof periodicTasks / sizeof *periodicTasks, sizeof *periodicTasks, timeperiod_cmp);
    qsort(aperiodicJobs, sizeof aperiodicJobs / sizeof *aperiodicJobs, sizeof *aperiodicJobs, arrival_cmp);
    int maxTimePeriod = periodicTasks[no_periodic-1].timeperiod;
    maxTime = MAX(maxTimePeriod,aperiodicJobs[no_aperiodic-1].arrival + Ts + aperiodicJobs[no_aperiodic-1].C);
    printf("%d\n",++maxTime);
    maxTime++;
//    serverbudget = (int*)malloc(maxTime*sizeof(int));
//    schedule = (int*)malloc(maxTime*sizeof(int));
    int i;
    for(i=0;i<no_periodic;i++)
    {
     printf("%d %d\n",periodicTasks[i].C, periodicTasks[i].timeperiod);

    }
    printf("computation and  arrival time aperiodic requests\n");
    for(i=0;i<no_aperiodic;i++)
    {
     printf("%d %d\n",aperiodicJobs[i].C, aperiodicJobs[i].arrival);
    }
    int aperiodic_counter =0;
    int t=0;
    while(t<maxTime){

        if(!replenishmentQ.empty()){
            struct replenishment tmp = (struct replenishment )replenishmentQ.front();
            if(t==tmp.RT){
                Cs += tmp.amount;
                replenishmentQ.pop();
                if(aperiodicJobs[aperiodic_counter].arrival<t && aperiodicJobs[aperiodic_counter].C>0){
                    struct jobsforQ newjob;
                    newjob.deadline =  t + Ts;
                    newjob.remainingC = min(aperiodicJobs[aperiodic_counter].C,Cs);
                    aperiodicJobs[aperiodic_counter].C -= newjob.remainingC;
                    newjob.type = 'a';
                    newjob.no = aperiodic_counter +1;
                    readyQ.push(newjob);
                    struct replenishment tmp;
                    tmp.amount = newjob.remainingC;
                    tmp.RT = t + Ts;
                    replenishmentQ.push(tmp);
                    if(aperiodicJobs[aperiodic_counter].C==0){
                        aperiodic_counter++;
                    }
                }
            }
        }

//        serverbudget[t]= Cs;
        //draw budget
        for(i=0;i<no_periodic;i++)
        {
            if(t%periodicTasks[i].timeperiod ==0){

                struct jobsforQ newjob;
                newjob.deadline = t + periodicTasks[i].timeperiod;
                newjob.remainingC = periodicTasks[i].C;
                newjob.type = 'p';
                newjob.no = i +1;
                readyQ.push(newjob);
            }


        }
        if(t==15)
        {
            cout<<"hi";
        }
        if(aperiodicJobs[aperiodic_counter].arrival==t){

            struct jobsforQ newjob;
            newjob.deadline =  t + Ts;
            newjob.remainingC = min(aperiodicJobs[aperiodic_counter].C,Cs);
            aperiodicJobs[aperiodic_counter].C -= newjob.remainingC;
            newjob.type = 'a';
            newjob.no = aperiodic_counter +1;
            readyQ.push(newjob);
            struct replenishment tmp;
            tmp.amount = newjob.remainingC;
            tmp.RT = t + Ts;
            replenishmentQ.push(tmp);
            if(aperiodicJobs[aperiodic_counter].C==0){
                aperiodic_counter++;
            }

        }

        if(t==30){

        cout<<"queue\n";
            while (!readyQ.empty())
              {
              struct jobsforQ tmp = (struct jobsforQ)(readyQ.top());
                 cout<<' '<<tmp.deadline;
                 readyQ.pop();
              }
              cout << '\n';
                    break;
        }


        if(!readyQ.empty()){
            struct jobsforQ tmp = (struct jobsforQ)(readyQ.top());

            if(tmp.deadline<t){
            cout<<"DEADLINE MISSED";
            }
            queue<struct jobsforQ> tmpQ;
            while(Cs==0&& tmp.type=='a'){

                tmpQ.push(tmp);
                readyQ.pop();
                if(!readyQ.empty()){
                    tmp =  (struct jobsforQ)(readyQ.top());
                }
                else
                    break;
            }
            if(!((Cs==0&& tmp.type=='a')))
                cout<<"t= "<<t<<" "<<tmp.type<<" "<<tmp.no<<" Cs ="<<Cs<<endl;
            if(tmp.type=='p'){
                tmp.remainingC--;
                readyQ.pop();

                if(tmp.remainingC>0 && tmp.deadline>=t)
                    readyQ.push(tmp);
            }
            else{
                if(Cs>0){
                //draw decreasing budget
                    Cs--;
                    tmp.remainingC--;
                    readyQ.pop();
                    if(tmp.remainingC>0 && tmp.deadline>=t)
                        readyQ.push(tmp);
                    }

            }

            while(!tmpQ.empty()){
                readyQ.push(tmpQ.front());
                tmpQ.pop();
            }


        }
        t++;

    }

}


int schedubilityTest() {
    printf("testing schedubility\n");
    U = U + (float)(Cs/Ts);

    if(U>1) return 0;
    else    return 1;

}

int takeinput(){

    printf("Enter no of periodic requests\n");
    scanf("%d",&no_periodic);
    printf("Enter no of aperiodic requests\n");
    scanf("%d",&no_aperiodic);
    printf("Enter: server budget, server timeperiod\n");
    scanf("%d %d",&Cs,&Ts);
    int i;
    periodicTasks = (struct periodic *)malloc(no_periodic*sizeof(struct periodic));
    aperiodicJobs = (struct aperiodic *)malloc(no_aperiodic*sizeof(struct aperiodic));
    printf("computation time and time period for periodic requests\n");
    for(i=0;i<no_periodic;i++)
    {
     scanf("%d %d",&periodicTasks[i].C, &periodicTasks[i].timeperiod);

    }
    printf("computation and  arrival time aperiodic requests\n");
    for(i=0;i<no_aperiodic;i++)
    {
     scanf("%d %d",&aperiodicJobs[i].C, &aperiodicJobs[i].arrival);
    }

    return 1;
}
