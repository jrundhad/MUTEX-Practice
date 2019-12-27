#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <fcntl.h>
#include <math.h>
#include <semaphore.h>

//create a structure for bank accounts
struct bankAcc{
  char name[20];
  char acctype[20];
  int depositFee;
  int withdrawFee;
  int transferFee;
  int transactionLimit;
  int transactionCount;
  int transactionFee;
  int overdraft;
  int overdraftFee;
  int balance;
  int overdraftMultiplier;
};
//declare gloabl variable of bank accounts array
struct bankAcc accounts[1000];
//declare lock and depositor and client funtions
pthread_mutex_t lock;
void  *depositorFunc(void *info);
void  *clientFunc(void *info);

//function for depositors
void  *depositorFunc(void *info){
  //lock the thread so no other threads can enter
  pthread_mutex_lock(&lock);  // ENTRY REGION
  // get the first token which just states the depositor name
  char * token = strtok(info, " ");

  //while loop to go thorugh till the end of the info line
  while (token != NULL){
    if(strcmp(token,"d")==0){
      //get the account name to deposit into
      token = strtok(NULL, " ");
      //find the account name in the accounts array
      int index=0;
      while(strcmp(accounts[index].name,token)!=0){
        index++;
      }
      //get the ammount to deposit
      token = strtok(NULL, " ");
      accounts[index].transactionCount = accounts[index].transactionCount + 1;
      //compare the transaction count with the limit and subtract the transaction fee accordingly
      if(accounts[index].transactionCount < accounts[index].transactionLimit){
        accounts[index].balance = accounts[index].balance + atoi(token);
        accounts[index].balance = accounts[index].balance - accounts[index].depositFee;
      } else {
        accounts[index].balance = accounts[index].balance + atoi(token);
        accounts[index].balance = accounts[index].balance - accounts[index].depositFee -  accounts[index].transactionFee;
      }
    }
    //get next token for while loop
    token = strtok(NULL, " ");
  }
  //unlock the lock to let the next thread enter
  pthread_mutex_unlock(&lock); // EXIT REGION
}

//function for clients
void  *clientFunc(void *info){
  pthread_mutex_lock(&lock); // ENTRY REGION
  //get the first word from the info
  char * token = strtok(info, " ");

  //while loop to go thorugh till the end of the info line
  while (token != NULL){
    //if the word is d then continue to deposit similar to deposit function above
    if(strcmp(token,"d")==0){
      token = strtok(NULL, " ");
      int index=0;
      while(strcmp(accounts[index].name,token)!=0){
        index++;
      }
      token = strtok(NULL, " ");
      accounts[index].transactionCount = accounts[index].transactionCount + 1;
      if(accounts[index].transactionCount < accounts[index].transactionLimit){
        accounts[index].balance = accounts[index].balance + atoi(token);
        accounts[index].balance = accounts[index].balance - accounts[index].depositFee;
      } else {
        accounts[index].balance = accounts[index].balance + atoi(token);
        accounts[index].balance = accounts[index].balance - accounts[index].depositFee -  accounts[index].transactionFee;
      }
    }
    // if the word is w then xontinue to withdraw
    if(strcmp(token,"w")==0){
      //get account name
      token = strtok(NULL, " ");
      //find account name in accounts array
      int index=0;
      while(strcmp(accounts[index].name,token)!=0){
        index++;
      }
      // get the amount to withdraw
      token = strtok(NULL, " ");
      int amount = atoi(token);
      //check if overdraft needs to be applied
      if (amount >  accounts[index].balance){
        //check to see if account has overdraft
        if (accounts[index].overdraft == 1 ){
          //checl what the the balance can be after subtracting the amount
          int newBalance =  accounts[index].balance - amount;
          //if the balance is greater than the 5000 overdraft limit
          if (newBalance >= -5000){
            //calculate the overdraft multiplier
            int Multiplier = ceil((float)accounts[index].balance/-500);
            //make sure the multiplier is different than the one stored in the struct
            if(Multiplier != accounts[index].overdraftMultiplier){
              //get the new multiplier which will be the one we apply to the overdraft fee
              int newMultiplier =  Multiplier -  accounts[index].overdraftMultiplier;
              //set new multiplier in struct
              accounts[index].overdraftMultiplier = Multiplier;
              //update the balance and subtract the fees and increment transaction count
              accounts[index].balance = accounts[index].balance -  amount;
              accounts[index].balance = accounts[index].balance - accounts[index].withdrawFee;
              accounts[index].balance = accounts[index].balance - (newMultiplier * accounts[index].overdraftFee);
              accounts[index].transactionCount = accounts[index].transactionCount +1;
              if(accounts[index].transactionCount > accounts[index].transactionLimit){
                accounts[index].balance = accounts[index].balance - accounts[index].transactionFee;
              }
            } else {
              //update the balance and subtract the fees and increment transaction count
              accounts[index].balance = accounts[index].balance -  amount;
              accounts[index].balance = accounts[index].balance - accounts[index].withdrawFee;
              accounts[index].transactionCount = accounts[index].transactionCount +1;
              if(accounts[index].transactionCount > accounts[index].transactionLimit){
                accounts[index].balance = accounts[index].balance - accounts[index].transactionFee;
              }
            }
          }
        }
      } else {
        //update the balance and subtract the fees and increment transaction count
        accounts[index].balance = accounts[index].balance -  amount;
        accounts[index].balance = accounts[index].balance - accounts[index].withdrawFee;
        accounts[index].transactionCount = accounts[index].transactionCount +1;
        if(accounts[index].transactionCount > accounts[index].transactionLimit){
          accounts[index].balance = accounts[index].balance - accounts[index].transactionFee;
        }
      }
    }
    //if the word is t continue to transfer
    if(strcmp(token,"t")==0){
      //get the first account name and find it in the accounts array and then get the second and do the same
      token = strtok(NULL, " ");
      int index=0;
      while(strcmp(accounts[index].name,token)!=0){
        index++;
      }
      token = strtok(NULL, " ");
      int index2 = 0;
      while(strcmp(accounts[index2].name,token)!=0){
        index2++;
      }
      //get transfer ammount
      token = strtok(NULL, " ");
      int amount = atoi(token);
      //do everything similar to withdraw above except instead of subtracting withdrawFee subtract the trasnfer fee
      if (amount >  accounts[index].balance){
        if (accounts[index].overdraft == 1 ){
          int newBalance =  accounts[index].balance - amount;
          if (newBalance >= -5000){
            int Multiplier = ceil((float)accounts[index].balance/-500);
            if(Multiplier != accounts[index].overdraftMultiplier){
              int newMultiplier =  Multiplier -  accounts[index].overdraftMultiplier;
              accounts[index].overdraftMultiplier = Multiplier;
              accounts[index].balance = accounts[index].balance -  amount;
              accounts[index].balance = accounts[index].balance - accounts[index].transferFee;
              //add amount to the second account and subtract the trafer fee as well as transaction fee if applicable
              accounts[index2].balance =  accounts[index2].balance + amount;
              accounts[index2].balance = accounts[index2].balance - accounts[index2].transferFee;
              accounts[index2].transactionCount = accounts[index2].transactionCount + 1;
              if(accounts[index2].transactionCount > accounts[index2].transactionLimit){
                accounts[index2].balance = accounts[index2].balance - accounts[index2].transactionFee;
              }
              accounts[index].balance = accounts[index].balance - (newMultiplier * accounts[index].overdraftFee);
              accounts[index].transactionCount = accounts[index].transactionCount +1;
              if(accounts[index].transactionCount > accounts[index].transactionLimit){
                accounts[index].balance = accounts[index].balance - accounts[index].transactionFee;
              }
            } else {
              accounts[index].balance = accounts[index].balance -  amount;
              accounts[index].balance = accounts[index].balance - accounts[index].transferFee;
              accounts[index].transactionCount = accounts[index].transactionCount +1;
              if(accounts[index].transactionCount > accounts[index].transactionLimit){
                accounts[index].balance = accounts[index].balance - accounts[index].transactionFee;
              }
              //add amount to the second account and subtract the trafer fee as well as transaction fee if applicable
              accounts[index2].balance =  accounts[index2].balance + amount;
              accounts[index2].balance = accounts[index2].balance - accounts[index2].transferFee;
              accounts[index2].transactionCount = accounts[index2].transactionCount + 1;
              if(accounts[index2].transactionCount > accounts[index2].transactionLimit){
                accounts[index2].balance = accounts[index2].balance - accounts[index2].transactionFee;
              }
            }
          }
        }
      } else {
        accounts[index].balance = accounts[index].balance -  amount;
        accounts[index].balance = accounts[index].balance - accounts[index].transferFee;
        accounts[index].transactionCount = accounts[index].transactionCount +1;
        if(accounts[index].transactionCount > accounts[index].transactionLimit){
          accounts[index].balance = accounts[index].balance - accounts[index].transactionFee;
        }
        //add amount to the second account and subtract the trafer fee as well as transaction fee if applicable
        accounts[index2].balance =  accounts[index2].balance + amount;
        accounts[index2].balance = accounts[index2].balance - accounts[index2].transferFee;
        accounts[index2].transactionCount = accounts[index2].transactionCount + 1;
        if(accounts[index2].transactionCount > accounts[index2].transactionLimit){
          accounts[index2].balance = accounts[index2].balance - accounts[index2].transactionFee;
        }
      }
    }
    // get the next word to pass into the while loop
    token = strtok(NULL, " ");
  }
  pthread_mutex_unlock(&lock); // EXIT REGION
}




int main(){
  char buf[1000];
  //open the input file
  int fd = open("assignment_3_input_file.txt", O_RDONLY);
  FILE *fl = fopen("assignment_3_input_file.txt","r");
  //intitialize x as bankacc struct
  struct bankAcc x;
  int accCount=0;
  //parse until end of file
  while(fscanf(fl, " %s ", buf) != EOF){
    //break the loop when deep1 is found
    if (strcmp (&buf,"dep1") == 0){
      break;
    }
    //set the name in struct
    strcpy(x.name,&buf);
    //ssanf until type of account is reached
    fscanf(fl, " %s ", buf);
    fscanf(fl, " %s ", buf);
    //store the type of the account in struct
    if (strcmp (&buf,"business") == 0){
      strcpy(x.acctype,&buf);
    } else if (strcmp(&buf, "personal")==0){
      strcpy(x.acctype,&buf);
    }
    //get the fees for deposit, withdraw and tranfer and store in struct
    fscanf(fl, " %s ", buf);
    fscanf(fl, " %s ", buf);
    x.depositFee =  atoi(&buf);
    fscanf(fl, " %s ", buf);
    fscanf(fl, " %s ", buf);
    x.withdrawFee = atoi(&buf);
    fscanf(fl, " %s ", buf);
    fscanf(fl, " %s ", buf);
    x.transferFee =  atoi(&buf);
    fscanf(fl, " %s ", buf);
    //get transaction limit and store in struct
    fscanf(fl, " %s ", buf);
    x.transactionLimit =  atoi(&buf);
    //get transaction fee and store in struct
    fscanf(fl, " %s ", buf);
    x.transactionFee =  atoi(&buf);
    fscanf(fl, " %s ", buf);
    //check if accoutn has overdraft
    fscanf(fl, " %s ", buf);
    //if accoutn has overdraft then update that in struct and also get and store the overdraft fee
    if (strcmp (&buf,"Y") == 0){
      x.overdraft = 1;
      fscanf(fl, " %s ", buf);
      x.overdraftFee = atoi(&buf);
    } else if (strcmp (&buf,"N") == 0){
      x.overdraft = 0;
      x.overdraftFee = 0;
    }
    //set everything else in struct to 0
    x.balance = 0;
    x.transactionCount = 0;
    x.overdraftMultiplier = 0;
    //add strcut to array of accounts and update account count
    accounts[accCount] = x;
    accCount = accCount+1;
  }
  //close the file
  fclose(fl);

  //open the file again
  FILE *fl2 = fopen("assignment_3_input_file.txt","r");
  // skipo through the account lines
  for(int j=0; j<accCount-1; j++){
    fscanf(fl2, "%[^\n]\n", buf);
  }
  //intitialize variables to keep count of depositors and clients
  int depCount=0;
  int cCount=0;
  while(fscanf(fl2, "%[^\n]\n", buf) != EOF){
    //if the first char of line is d then increment deopistor count and similar for client if char is c
    fgets(buf, 2, fl2);
    if(strcmp (&buf,"d")==0){
      depCount=depCount+1;
    } else if (strcmp (&buf,"c")==0){
      cCount=cCount+1;
    }
  }
  //close the file
  fclose(fl2);

  //create array to store the depositor lines
  char depArr[depCount][1000];

  FILE *fl3 = fopen("assignment_3_input_file.txt","r");
  //scan through the account lins
  for(int j=0; j<accCount; j++){
    fscanf(fl3, "%[^\n]\n", buf);
  }
  //scan the amount of depositor  lines
  for(int x=0; x<depCount; x++){
    //store the line in deparr
    fscanf(fl3, "%[^\n]\n", buf);
    strcpy(depArr[x],buf);
  }
  fclose(fl3);



  //intitialize the threads and mutex
  int i, err_thread;
  pthread_t depThreads[depCount];
  if (pthread_mutex_init(&lock, NULL) != 0)
      {
          printf("\n mutex init failed\n");
          return 1;
      }
  //create a thread for each depositor
  for (i = 0; i< depCount; i++)
  {
    err_thread = pthread_create(&depThreads[i], NULL, &depositorFunc, &depArr[i]);

    if (err_thread != 0)
      printf("\n Error creating thread %d", i);
  }
  //wait for depositor threads to finish
  for (i = 0; i< depCount; i++)
      pthread_join(depThreads[i], NULL);
  //destroy the lock
  pthread_mutex_destroy(&lock);



  //open the file again but this time store all the client information into the clientArr
  FILE *fl4 = fopen("assignment_3_input_file.txt","r");
  char clientArr[cCount][1000];
  for(int r=0; r<accCount+depCount; r++){
    fscanf(fl4, "%[^\n]\n", buf);
  }
  for(int k=0; k<cCount; k++){
      fscanf(fl3, "%[^\n]\n", buf);
      strcpy(clientArr[k],buf);
  }
  fclose(fl4);

  //intitialize the number of client threads and mutex
  pthread_t clientThreads[cCount];
  int j;
  if (pthread_mutex_init(&lock, NULL) != 0)
      {
          printf("\n mutex init failed\n");
          return 1;
      }

  //create the client threads and execute them
  for (j = 0; j < cCount; j++)
  {
    err_thread = pthread_create(&clientThreads[j], NULL, &clientFunc, &clientArr[j]);

    if (err_thread != 0)
      printf("\n Error creating thread %d", j);
  }
  //wait for client threads to finish
  for (j = 0; j< cCount; j++)
      pthread_join(clientThreads[j], NULL);

  pthread_mutex_destroy(&lock);


  //print the account information
  FILE * fp;
  fp = fopen ("assignment_3_output_file.txt","w");
  for (int x=0 ;  x < accCount ; x++){
    printf("%s type %s %d\n", accounts[x].name, accounts[x].acctype, accounts[x].balance);
    fprintf(fp, "%s type %s %d\n", accounts[x].name, accounts[x].acctype, accounts[x].balance );
  }
  fclose(fp);
}
