# include <stdlib.h>  // included the required header files
# include <iostream>
# include <fstream>
# include <vector>
# include <thread>
# include <semaphore.h>
#include <sys/time.h>

std::ofstream file2; // for writing into the file
int n_threads,n; // 
sem_t lock; // semaphore lock
struct node  // it contains data of all the vertices
{
	int vertex;  // stores the vertex number
	int colour; // stores the colour of the vertex
	int partition; // it stores the partition in which this vertex beongs to 
	char type; // it tells weather the vertex in internal or external 
	struct node*next; // pointer to link
	bool *colour_array;  // helpful while colouring
};


struct pnode // for threads
{
	int vertex; // if tells the vertices which are under control of the thread
	struct pnode*next;
};

struct node**array; // array of vertices
struct pnode**parray; // array of threads
void print(int n);
void pprint(int n);
void add(struct node**head,int value) // it is an insertion of a node for vertices array
{
	struct node*temp1=*head;
	struct node*temp=(struct node*)malloc(sizeof(struct node));
	temp-> vertex= value;
	temp->next=NULL;
	while(temp1->next !=NULL)
	{
		temp1=temp1->next;
	}
	temp1->next=temp;
}

struct node*create_node(int i,int n) // allocating memory for the node
{
	struct node*temp=(struct node*)(malloc(sizeof(struct node)));
	temp->vertex=i;
	temp->colour=-1;
	temp->next=NULL;
	temp->colour_array= new bool[n];
	return temp;
}

void padd(struct pnode**head,int value) // it is an insertion of a node for thread array
{
	struct pnode*temp1=*head;
	struct pnode*temp=(struct pnode*)malloc(sizeof(struct pnode));
	temp-> vertex= value;
	temp->next=NULL;
	while(temp1->next !=NULL)
	{
		temp1=temp1->next;
	}
	temp1->next=temp;
}

void colouring() // here it is the main algorithm for colouring
{
	struct pnode*temp=parray[id]; 
	temp=temp->next;
	while(temp!=NULL)
	{
		int cv= temp->vertex; // cv means current vertex
		if(array[cv]->type == 'i')// if the vertex is internal
		{
			struct node*temp1=array[cv]; // 
			temp1=temp1->next;
			while(temp1!=NULL) 
			{
				int cn=temp1->vertex; // cn stands for current neighbour vertex
				if(array[cn]->colour!=-1) array[cv]->colour_array[array[cn]->colour]=true;
				temp1=temp1->next;	// knowing the colours that are occupied by the neighbouring vertices
			}
			int k=0,i=0;
			while(k == 0 && i<n) // assigning the least unoccupied colour
			{
				if(array[cv]->colour_array[i]== false) 
					k=1;	
				if(k==1) array[cv]->colour = i;
				i++;

			}
		}
		else  // if the vetex is an external vertex
		{
			sem_wait(&lock);	// lock allows only one thread to print at a time 
			struct node*temp1=array[cv];
			temp1=temp1->next;
			while(temp1!=NULL)
			{
				int cn=temp1->vertex; // cn stands for current neighbour vertex
				if(array[cn]->colour!=-1) array[cv]->colour_array[array[cn]->colour]=true;
				temp1=temp1->next; 	// knowing the colours that are occupied by the neighbouring vertices
			}
			int k=0,i=0;
			while(k == 0 && i<n) // assigning the least unoccupied colour
			{
				if(array[cv]->colour_array[i]== false)
					k=1;
				if(k==1) array[cv]->colour = i;
				i++;
			}
			sem_post(&lock); // removing lock
		}

		temp=temp->next;
	}
	
}

int main()
{
	std::ifstream file1;
	file1.open("input_params.txt"); // opening input.txt
	file2.open("output_coarse_Lock.txt"); // opening output file
	sem_init(&lock,0,1);   // initializing locks
	if(!file1)
	{ // checking file existence
		std::cout << "no such file\n"; 
		return 0;
	}
	file1 >> n_threads >> n;
	array=(struct node**)malloc((n+1)*sizeof(struct node*)); //creating array of pointers dynamically 
	if(array == NULL ) std::cout << "memory not allocated"; // if malloc was unable to allocate memory
	for(int i=1;i<=n;i++)
		array[i]=create_node(i,n); // allocating memory
	int j,k;
	char ch;
	while(!file1.eof()) // reading from the file
	{
		file1 >> j ;
		file1.get(ch);
		while(ch!= '\n' && !file1.eof())
		{
			file1 >> k;
			add(&array[j],k); // adding adjacent vertex at the end of the linkedlist 
			file1.get(ch);		
		}
	}
	file1.close();
	for(int i=1;i<=n;i++)
		array[i]->partition=(rand()%n_threads)+1; // every thread is allocating a random partition
	parray=(struct pnode**)malloc((n_threads+1)*sizeof(struct node*)); // creating array of thread pointers dynamically 
	if(parray == NULL ) std::cout << "memory not allocated";
	for(int i=1;i<=n_threads;i++) // allocaing memory for each pnode
	{
		parray[i]=(struct pnode*)(malloc(sizeof(struct pnode)));
		parray[i]->next = NULL;
		parray[i]->vertex=i;
	}
	for(int i=1;i<=n;i++) 
		padd(&parray[array[i]->partition],i); // adding the vertex at the end of the linkedlist 
								// getting internal and external nodes
	for(int i=1;i<=n_threads;i++) // looping through thread array
	{
		struct pnode*temp=parray[i]; 
		while(temp->next!=NULL)
		{
			temp=temp->next;
			struct node*temp1=array[temp->vertex];
			int flag=0; // indication for node type
			while(temp1->next!=NULL)
			{
				temp1=temp1->next;
				if(array[temp1->vertex]->partition != i)
					flag=1;
			}
			if(flag==0) array[temp->vertex]->type='i'; // assigning its type
			else  array[temp->vertex]->type='e';
		}
	}
	
	std::vector<std::thread> threads; // creating threads
	struct timeval start,end;   // to calculate the average waiting time in microseconds
	gettimeofday(&start, NULL); // getting time from epoch till now in mill seconds and is stored in start
	colouring();
  	gettimeofday(&end, NULL); // getting time from epoch till now in mill seconds and is stored in end
  	int n_colours=0; // stores the no of colours used
  	for(int i=1;i<=n;i++)  // no of colours calculation
  	{
  		if(array[i]->colour > n_colours)
  			n_colours = array[i]->colour; 
    }					// printing the required values
    file2 << "coarse Lock" << std::endl;
  	file2 << "No. of colours used: " << n_colours+1 <<std::endl;
  	file2 <<  "Time taken by the algorithm: " << (float)((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec))/1000<< "milliseconds" << std::endl;   // calculating time taken in milliseconds                                       	
  	file2 << "colours:\n" ;
  	for(int i=1;i<=n;i++)
  		file2 << "v" << i << " - " << array[i]->colour+1 << ","; 
	
	return 0;
}