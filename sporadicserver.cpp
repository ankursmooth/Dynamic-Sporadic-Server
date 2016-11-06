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
ALLEGRO_DISPLAY *display ;
    ALLEGRO_FONT *font ;
int no_periodic,no_aperiodic;
int Ts,Cs,RA,RT,ds;
map<int,int>positionLines;
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

void drawBasic();
int takeinput();

int schedubilityTest() ;

int main(int argc, char **argv)
{
    printf("Dynamic Sporadic Server\n");
    if(!takeinput()) {
      fprintf(stderr, "failed to takeinput!\n");
      return -1;
    }
    schedubilityTest();
    cout<<"U = "<<U<<endl;
    calculateSchedule();
    al_destroy_display(display);

}


void calculateSchedule(){
    int c;
    cout<<" press any key to continue\n";

    cin>>c;

    qsort(periodicTasks, sizeof periodicTasks / sizeof *periodicTasks, sizeof *periodicTasks, timeperiod_cmp);
    qsort(aperiodicJobs, sizeof aperiodicJobs / sizeof *aperiodicJobs, sizeof *aperiodicJobs, arrival_cmp);
    int maxTimePeriod = periodicTasks[no_periodic-1].timeperiod;
    maxTime = MAX(maxTimePeriod,aperiodicJobs[no_aperiodic-1].arrival + Ts + aperiodicJobs[no_aperiodic-1].C);
    printf("%d\n",++maxTime);
    maxTime++;
    drawBasic();
//    serverbudget = (int*)malloc(maxTime*sizeof(int));
//    schedule = (int*)malloc(maxTime*sizeof(int));
    int i;
    printf("computation and  time period of periodic \n");
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

        al_rest(0.1);
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
                al_draw_line(75+ t*22,positionLines[i+2],75+ t*22,positionLines[i+2]-25,al_map_rgb(i*70,i*40,i*10),1);
            }


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
            al_draw_line(75+ t*22,positionLines[1],75+ t*22,positionLines[1]-25,al_map_rgb(0,200,10),1);

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
            if(!((Cs==0&& tmp.type=='a'))){
                cout<<"t= "<<t<<" type "<<tmp.type<<" number = "<<tmp.no<<" Cs ="<<Cs<<endl;
                if(tmp.type=='a')
                    al_draw_filled_rectangle(75+t*22,positionLines[1],75+(t+1)*22,positionLines[1]-15,al_map_rgb(10,10,10));
                else
                    al_draw_filled_rectangle(75+t*22,positionLines[1+tmp.no],75+(t+1)*22,positionLines[1+tmp.no]-15,al_map_rgb((130*tmp.no)%255,(10*tmp.no)%255,(200*tmp.no)%255));
                }

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
                    al_draw_filled_triangle(75+(t)*22,positionLines[0]-Cs*15, 75+(t)*22,positionLines[0]-(Cs+1)*15,75+(t+1)*22,positionLines[0]-Cs*15,al_map_rgb(50,100,150));
                    //al_draw_filled_rectangle(75 +t*22,positionLines[0],75 +(t+1)*22,positionLines[0] - Cs*15,al_map_rgb(50,100,150));
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
        al_draw_filled_rectangle(75 +t*22,positionLines[0],75 +(t+1)*22,positionLines[0] - Cs*15,al_map_rgb(50,100,150));
        t++;
        al_flip_display();
        al_rest(1.5);

    }

    al_flip_display();
    al_rest(5);
    cout<<"\n press any key to continue\n";

    cin>>c;
}

void drawBasic(){
    display = NULL;

        if(!al_init()) {
            //return -1;
            fprintf(stderr, "failed to initialize allegro!\n");
        }
        al_init_primitives_addon();
        display = al_create_display(SCREEN_W, SCREEN_H);
        al_clear_to_color(al_map_rgb(255,255,255));
        if(!display) {
            //return -1;
            fprintf(stderr, "failed to create display!\n");
        }
     al_flip_display();
     al_rest(1.0);

        al_init_font_addon();
        al_init_ttf_addon();
        font= al_load_font ("pirulen.ttf" , 7 , 0);
         if (!font){
      fprintf(stderr, "Could not load 'pirulen.ttf'.\n");
//      return -1;
   }
        int offset =60;
        al_draw_text(font,al_map_rgb(0,0,0),2,10*Cs +offset-20,0,"Server");
        al_flip_display();
        al_rest(1.0);
        positionLines[0] = 10*Cs +offset;

        al_draw_line(75, positionLines[0] ,600,  positionLines[0]  ,al_map_rgb(0,0,0),1.0);
        for(int j =0;j<=maxTime;j++){
                al_draw_filled_circle(75 + j*22, positionLines[0]+1,1.5,al_map_rgb(120,120,120));
            }

        al_flip_display();
        al_draw_text(font,al_map_rgb(0,0,0),2,10 +50*(1) +offset-20,0,"aperiodic");
        al_flip_display();
        al_rest(1.0);
        positionLines[1] = 10*Cs  +offset + 50*(1);
        al_draw_line(75, positionLines[1] ,600,  positionLines[1]  ,al_map_rgb(0,0,0),1.0);
        for(int j =0;j<=maxTime;j++){
                al_draw_filled_circle(75 + j*22, positionLines[1]+1,1.5,al_map_rgb(120,120,120));
            }

        for(int i=0;i<no_periodic;i++)
        {

            al_draw_textf(font,al_map_rgb(0,0,0),2,10 +50*(i+2) +offset-20,0,"periodic %d",i+1);
            al_flip_display();
            al_rest(1.0);
            positionLines[i+2]= 10*Cs  +offset + 50*(i+2);
            al_draw_line(75,positionLines[i+2],600, positionLines[i+2] ,al_map_rgb(0,0,0),1.0);
            for(int j =0;j<=maxTime;j++){
                al_draw_filled_circle(75 + j*22, positionLines[i+2]+1,1.5,al_map_rgb(120,120,120));
            }


        }

        al_flip_display();
    al_rest(1.0);



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
    printf("Enter: server budget, server timeperiod Ts\n");
    scanf("%d %d",&Cs,&Ts);
    int i;
    periodicTasks = (struct periodic *)malloc(no_periodic*sizeof(struct periodic));
    aperiodicJobs = (struct aperiodic *)malloc(no_aperiodic*sizeof(struct aperiodic));
    printf("Enter computation time and time period for periodic requests\n");
    for(i=0;i<no_periodic;i++)
    {
     scanf("%d %d",&periodicTasks[i].C, &periodicTasks[i].timeperiod);

    }
    printf("Enter computation and  arrival time aperiodic requests\n");
    for(i=0;i<no_aperiodic;i++)
    {
     scanf("%d %d",&aperiodicJobs[i].C, &aperiodicJobs[i].arrival);
    }

    return 1;
}
