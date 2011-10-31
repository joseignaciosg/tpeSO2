/********************************** 
*
*  kernel.c
*  	Galindo, Jose Ignacio
*  	Homovc, Federico
*		ITBA 2011
*
***********************************/


/***	Project Includes	***/
#include "../include/kasm.h"
#include "../include/kernel.h"
#include "../include/shell.h"
#include "../include/utils.h"
#include "../include/process.h"
#include "../include/fs.h"


DESCR_INT idt[0x90]; /* IDT 144 positions*/
IDTR idtr;			 /* IDTR */

PROCESS idle;
processList ready;
int nextPID = 1;
int CurrentPID = 0;
int currentTTY = 0;
int currentProcessTTY = 0;
int logPID;
TTY terminals[4];
user admin;

extern int timeslot;
extern int logoutPID;
extern int actualKilled;
extern int last100[100];
extern int usrLoged;
extern int usrName;
extern int password;
semItem * semaphoreTable;
int semCount;

/*for testing fifos*/
#define MAX_FIZE_SIZE 100
#define MAX_FIFO 100
my_fdItem * myfd_table;
int fileCount;

void
initializeSemaphoreTable(){
	semaphoreTable = malloc(sizeof(semItem)*20); /* UP TO 20 semaphores */
	semCount = 0;
	myfd_table = malloc(sizeof(my_fdItem)*MAX_FIFO); /* UP TO MAX_FIFO tables */
	fileCount = 0;
}

/*for testing fifos*/
int create_file(){
	myfd_table[fileCount].fd = fileCount;/*cabeza*/
	myfd_table[fileCount].file = malloc(sizeof(char)*MAX_FIZE_SIZE);
	myfd_table[fileCount].curr_size =0;
	semItem * sem = malloc(sizeof(semItem));
	sem->value = 0;
	semget_in_kernel(sem);
	myfd_table[fileCount].sem_key = sem->key;
	if (myfd_table[fileCount].sem_key == -1){
		printf("Not enouth space for more semaphores\n");
		return -1;
	}
	return myfd_table[fileCount++].fd;
}

void write_fifo(int fd, char *buf, int n){
	/*buscar el inodo correspondiente al fd
	*aumentar el semaforo correpondiente tantas
	*unidades como bytes se escriban
	*/
	/*improve!*/
	int j;
	if ( n > (MAX_FIZE_SIZE - myfd_table[fd].curr_size) ){
		memcpy(myfd_table[fd].file,buf ,MAX_FIZE_SIZE - myfd_table[fd].curr_size);
		for(j=0; j<MAX_FIZE_SIZE - myfd_table[fd].curr_size; j++){
			up_in_kernel(myfd_table[fd].sem_key);
		}
		myfd_table[fd].curr_size = MAX_FIZE_SIZE;
	}else{
		memcpy(myfd_table[fd].file, buf ,n);
		for(j=0; j<n; j++){
			up_in_kernel(myfd_table[fd].sem_key);
		}
		myfd_table[fd].curr_size += n;
	}
}

void read_fifo(int fd, char *buf, int n){
	int j;
	char * auxbuf;
	if ( n > MAX_FIZE_SIZE ){
		n = MAX_FIZE_SIZE;
	}
	for (j=0; j<n ; j++){
		down_in_kernel(myfd_table[fd].sem_key);/*blocking or not*/
	}
	memcpy( buf, myfd_table[fd].file , n );

	/*delete from file all the data read*/
	auxbuf = malloc(myfd_table[fd].curr_size-n);
	memcpy( myfd_table[fd].file, buf , myfd_table[fd].curr_size-n );
	myfd_table[fd].curr_size -= n;

}


void
initializeIDT()
{
	setup_IDT_entry (&idt[0x08], 0x08, (dword)&_int_08_hand, ACS_INT, 0);
	setup_IDT_entry (&idt[0x09], 0x08, (dword)&_int_09_hand, ACS_INT, 0);
	setup_IDT_entry (&idt[0x80], 0x08, (dword)&_int_80_hand, ACS_INT, 0);
	setup_IDT_entry (&idt[0x79], 0x08, (dword)&_int_79_hand, ACS_INT, 0);/*for block_process and kill*/
	idtr.base = 0;
	idtr.base += (dword)&idt;
	idtr.limit = sizeof(idt) - 1;
	_lidt (&idtr);
}


void
unmaskPICS(){
	_mascaraPIC1(0xFC);
   	_mascaraPIC2(0xFF);
}


void
setup_IDT_entry (DESCR_INT *item, byte selector, dword offset, byte access,
			 byte cero) {
  item->selector = selector;
  item->offset_l = offset & 0xFFFF;
  item->offset_h = offset >> 16;
  item->access = access;
  item->cero = cero;
  
  return;
}


void
reboot(){
	_export( 0x64, 0xFE); /* pulse CPU reset line */
	return;
}

/**********************************************
Starting point of the whole OS
*************************************************/
int
kmain()
{
	int i;
	_Cli();
	k_clear_screen();

	initializeSemaphoreTable(); /*brand new!*/
	initializeIDT();
	unmaskPICS();
	SetupScheduler();

	/*int h;
	char * buffer = calloc(512,512);
	char * read = calloc(512,512);

	for( h=0;h<(512*512);h++){
			buffer[h] = '1';
	}
	buffer[h] = '\0';	
	printf("%s",buffer);	
	h = 0;
	write_disk(0,h,buffer,(512*512),0);
	printf("Escribio\n");
	for(h=0;h<16024*16024;h++);	
	read_disk(0,h,read,(512*512),0);
	printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
printf("READ\n");
	for(h=0;h<16024*16024;h++);		
	printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
		
	printf("%s",read);
	//}*/
	int h;
	char * buffer = calloc(512,1);
	for(h=0;h<200;h++){
		write_disk(0,h,buffer,BLOCK_SIZE,0);
	}
	fd_table = (filedescriptor *)calloc(100,1);
	masterBootRecord * mbr = (masterBootRecord *)malloc(512);
	superblock = (masterBlock*)malloc(512);		
	bitmap = (BM*)calloc(BITMAP_SIZE,1);	
	inodemap = (IM*)calloc(INODEMAP_SIZE,1);
	//printf("PASO\n");
	
	read_disk(0,0,mbr,BLOCK_SIZE,0);

	if ( mbr->existFS == 0 ){
		init_filesystem("Chinux", mbr);
	}else{
		load_filesystem();
	}
	
	/*printf("mbr:%d\n",mbr->existFS);
	read_disk(0,1,superblock,BLOCK_SIZE,0);
	printf("name:%s\nblock:%d\nfreeBlocks:%d\nusedBlocks:%d\n",superblock->name, superblock->blockSize, superblock->freeBlocks, superblock->usedBlocks);
	printf("InodeSize:%d\n",sizeof(iNode));
	printf("Directory:%d\n",sizeof(directoryEntry));//16 Directorios o archivos en bloques directos..*/	
	//makeDir("Lala");
	//makeDir("ASD");
	
	ready = NULL;
	for(i = 0; i < 4; i++)
		startTerminal(i);
	logPID = CreateProcessAt("Login", (int(*)(int, char**))logUser, 0, 0, (char**)0, 0x400, 5, 1);
	strcopy(admin.name, "chinux", str_len("chinux"));
	strcopy(admin.password, "chinux", str_len("chinux"));
	_Sti();

	while(TRUE)
	;
	return 1;
}

PROCESS * GetProcessByPID(int pid)
{
	processNode * aux;
	if(pid == 0 || ready == NULL)
		return &idle;

	aux = ((processNode*)ready);
	while(aux != NULL && aux->process->pid != pid)
		aux = ((processNode*)aux->next);
	if(aux == NULL)
		return &idle;
	return aux->process;
}


int CreateProcessAt_in_kernel(createProcessParam * param)
{
	PROCESS * proc;
	void * stack = malloc(param->stacklength);
	proc = malloc(sizeof(PROCESS));
	proc->name = (char*)malloc(15);
	proc->pid = nextPID;
	proc->foreground = param->isFront;
	proc->priority = param->priority;
	memcpy(proc->name, param->name,str_len(param->name) + 1);
	proc->state = READY;
	proc->tty = param->tty;
	proc->stacksize = param->stacklength;
	proc->stackstart = (int)stack;
	proc->ESP = LoadStackFrame(param->process,param->argc,param->argv,(int)(stack + param->stacklength - 1), end_process);
	proc->parent = CurrentPID;
	proc->waitingPid = 0;
	proc->sleep = 0;
	proc->acum = param->priority + 1;
	set_Process_ready(proc);
	return proc->pid;

}



int LoadStackFrame(int(*process)(int,char**),int argc,char** argv, int bottom, void(*cleaner)())
{
	STACK_FRAME * frame = (STACK_FRAME*)(bottom - sizeof(STACK_FRAME));
	frame->EBP = 0;
	frame->EIP = (int)process;
	frame->CS = 0x08;
	frame->EFLAGS = 0;
	frame->retaddr = cleaner;
	frame->argc = argc;
	frame->argv = argv;
	return (int)frame;
}

void set_Process_ready(PROCESS * proc)
{
	processNode * node;
	processNode * aux;
	node = malloc(sizeof(processNode));
	aux = malloc(sizeof(processNode));
	node->next = NULL;
	node->process = proc;
	if(ready == NULL)
	{
		ready = (processList)node;
		return;
	}

	aux = ((processNode*)ready);
	while(aux->next != NULL)
		aux = (processNode*)aux->next;
	aux->next = (processList)node;
	return;
}

void block_process_in_kernel(int pid)
{
	processNode * aux;

	timeslot = 1;					/*"yield"*/
	if(ready == NULL)
	{
		_yield();
		return;
	}
	
	//printf("blocking process: %d\n", pid);
	aux = ((processNode*)ready);
	if(aux->process->pid == pid)
	{
		aux->process->state = BLOCKED;
	}else{
		while(aux->next != NULL && ((processNode*)aux->next)->process->pid != pid)
			aux = ((processNode*)aux->next);
		if(aux->next == NULL)
		{
			_yield();
			return;
		}
		((processNode*)aux->next)->process->state = BLOCKED;
	}
	_yield();
	return;
}

/*awakes the process with the given pid*/
void awake_process(int pid)
{
	PROCESS * proc;

	proc = GetProcessByPID(pid);
	if(proc->state == BLOCKED && !proc->waitingPid)
	{
		proc->state = READY;
		//printf("awakening process: %d\n", pid);
	}
	return ;
}



int Idle(int argc, char* argv[])
{
	_Sti();
	while(1)
	;
}

void end_process(void)
{
	PROCESS * proc;
	PROCESS * parent;
	processNode * aux;
	int i;
	
	_Cli();
	actualKilled = 1;
	proc = GetProcessByPID(CurrentPID);
	//printf("ending process: %d\n", proc->pid);
	if(!proc->foreground)
		printf("[%d]\tDone\t%s\n", proc->pid, proc->name);
	else{
		parent = GetProcessByPID(proc->parent);
		if(parent->waitingPid == proc->pid)
		{
			parent->waitingPid = 0;
			awake_process(parent->pid);
		}
	}

	aux = ((processNode *)ready);
	if(aux->process->pid == CurrentPID)
		ready = aux->next;
	else {
		while(aux->next != NULL && ((processNode *)aux->next)->process->pid != CurrentPID)
			aux = ((processNode *)aux->next);
		if(aux->next !=  NULL)
			aux->next = ((processNode*)aux->next)->next;
	}

	for(i = 0; i < 100; i++)
		if(last100[i] == proc->pid)
			last100[i] = -1;

	_Sti();

	return ;
}

void kill_in_kernel(int pid)
{
	PROCESS * proc;
	PROCESS * parent;
	processNode * aux;
	int i;

	_Cli();
	if(pid == 0 || pid == logoutPID || pid == logPID)
	{
		_Sti();
		return;
	}
	if(pid == CurrentPID)
		actualKilled = 1;
	//printf("killing %d\n", pid);
	proc = GetProcessByPID(pid);
	if(proc->foreground){
		parent = GetProcessByPID(proc->parent);
		if(parent->waitingPid == proc->pid)
		{
			parent->waitingPid = 0;
			awake_process(parent->pid);
		}
	}

	aux = ((processNode*)ready);
	while(aux != NULL)
	{
		if(aux->process->parent == pid)
			kill(aux->process->pid);
		aux = ((processNode*)aux->next);
	}

	aux = ((processNode *)ready);
	if(aux->process->pid == pid)
		ready = aux->next;
	else {
		while(aux->next != NULL && ((processNode *)aux->next)->process->pid != pid)
			aux = ((processNode *)aux->next);
		if(aux->next !=  NULL)
			aux->next = ((processNode*)aux->next)->next;
	}

	for(i = 0; i < 100; i++)
		if(last100[i] == proc->pid)
			last100[i] = -1;

	_Sti();

	return ;
}

void clearTerminalBuffer_in_kernel( int ttyid){
	terminals[ttyid].buffer.first_char = terminals[ttyid].buffer.actual_char + 1 % BUFFER_SIZE;
	terminals[ttyid].buffer.size = 0;
}

void waitpid_in_kernel(int pid)
{
	PROCESS* proc;
	proc = GetProcessByPID(CurrentPID);
	proc->waitingPid = pid;
	block_process(CurrentPID);
}

void getTerminalSize_in_kernel(int * size){
	(*size) = terminals[currentTTY].buffer.size;
}

void getTerminalCurPos_in_kernel(int * curpos){
	(*curpos) = terminals[currentProcessTTY].curpos;
}

void getCurrentTTY_in_kernel(int * currtty ){
	(*currtty) = currentTTY;
}


void mkfifo_in_kernel(fifoStruct * param){
	/*TODO*/
	/* open 2 file
	 * create 2 semathore
	 * return param
	 * */
	int fd1, fd2;
	fd1 = create_file();
	fd2 = create_file();
	param->fd1 = fd1;/*create files / use param->path*/
	param->fd2 = fd2;

}


void
semget_in_kernel(semItem * param){
		if ( semCount == 20 ){
			param->status = -1; /*failed*/
			return;
		}
		param->key = semCount;
		param->status = 0; /*ok*/
		param->blocked_proc_pid = -1; /*ok*/
		semaphoreTable[semCount++] = (*param);
}

void
up_in_kernel(int key){
	/*if the process is blocked-> unblock*/
	PROCESS * proc = GetProcessByPID(semaphoreTable[key].blocked_proc_pid);
	if (proc->state = BLOCKED){
		awake_process(proc->pid);
	}
	semaphoreTable[key].value++;
}

void
down_in_kernel(int key){
	if (semaphoreTable[key].value == 0){
		semaphoreTable[key].blocked_proc_pid = CurrentPID;
		block_process_in_kernel(CurrentPID);
	}
	semaphoreTable[key].value--;
}




void int_79(size_t call, size_t param){
	switch(call){
	case CREATE:/* create function */
		CreateProcessAt_in_kernel((createProcessParam *)param);
		break;
	case KILL: /* kill function */
		kill_in_kernel(param);/*param == pid*/
		break;
	case BLOCK:/* block function */
		block_process_in_kernel(param);/*param == pid*/
		break;
	case CLEAR_TERM:
		clearTerminalBuffer_in_kernel(param); /*param == ttyid*/
		break;
	case WAIT_PID:
		waitpid_in_kernel(param); /*param == pid*/
		break;
	case TERM_SIZE:
		getTerminalSize_in_kernel((int *)param); /*param == *size*/
		break;
	case TERM_CURPOS:
		getTerminalCurPos_in_kernel((int *)param); /*param == *curpos*/
		break;
	case CURR_TTY:
		getCurrentTTY_in_kernel((int *)param); /*param == *currtty*/
		break;
	case MK_FIFO:
		mkfifo_in_kernel((fifoStruct *)param);
		break;
	case SEM_GET:
		semget_in_kernel((semItem *)param);
		break;
	case SEM_UP:
		up_in_kernel(param); /*param == key*/
		break;
	case SEM_DOWN:
		down_in_kernel(param);/*param == key*/
		break;
	case MK_DIR:
		makeDir((char *)param);/*param == nameDIR*/
		break;
	case LS_COM:
		ls_in_kernel((char *)param);/*param == path*/
		break;
	case RM_COM:
		rmDir((char *)param);/*param == path*/
		break;
	case TOUCH_COM:
		touch_in_kernel((char *)param);/*param == filename*/
		break;
	case CAT_COM:
		cat_in_kernel((char *)param);/*param == filename*/
		break;
	case CD_COM:
		cd_in_kernel((char *)param);/*param == path*/
		break;
	case LINK_COM:
		link_in_kernel((link_struct *)param);
		break;
	case CREAT_COM:
		creat_in_kernel((creat_param *)param);
		break;
	}


}

void startTerminal(int pos)
{
	int i;
	for( i = 0; i < 80*25*2; i++)
	{
		terminals[pos].terminal[i]=' ';
		i++;
		terminals[pos].terminal[i] = WHITE_TXT;
	}
	terminals[pos].buffer.actual_char = BUFFER_SIZE-1;
	terminals[pos].buffer.first_char = 0;
	terminals[pos].buffer.size = 0;
	//terminals[pos].PID = pos + 1;
}

void sleep(int secs)
{
	PROCESS * proc;
	proc = GetProcessByPID(CurrentPID);
	proc->sleep = 18 * secs;
	block_process(CurrentPID);
}

void logUser(void)
{
	int i;
	while(!usrLoged)
	{
		printf("username: ");
		moveCursor();
		usrName = 1;
		block_process(CurrentPID);
		parseBuffer();
		usrName = 0;
		printf("\n");
		printf("password: ");
		moveCursor();
		password = 1;
		block_process(CurrentPID);
		parseBuffer();
		password = 0;
		printf("\n");
	}
	terminals[0].PID = CreateProcessAt("Shell0", (int(*)(int, char**))shell, 0, 0, (char**)0, 0x400, 2, 1);
	terminals[1].PID = CreateProcessAt("Shell1", (int(*)(int, char**))shell, 1, 0, (char**)0, 0x400, 2, 1);
	terminals[2].PID = CreateProcessAt("Shell2", (int(*)(int, char**))shell, 2, 0, (char**)0, 0x400, 2, 1);
	terminals[3].PID = CreateProcessAt("Shell3", (int(*)(int, char**))shell, 3, 0, (char**)0, 0x400, 2, 1);
	_Sti();
	return;
}


void logout(int argc, char * argv[])
{
	int i;
	for(i = 0; i < 4; i++)
		kill(terminals[i].PID);
	usrLoged = 0;
	logPID = CreateProcessAt("logUsr", (int(*)(int, char**))logUser, currentProcessTTY, 0, (char**)0, 0x400, 4, 1);
	_Sti();
}

