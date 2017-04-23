#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <vector>
#include <algorithm>

using namespace std;

#pragma warning(disable:4996) 
#define BUFFER 20000
#define MAXITEMNUM 20000//the maxinum number of items, items whose indexes higher than it will be ignored
double Items[MAXITEMNUM]= {0};//the array recording the support of every item
int itemsize = 0;
int patternNum = 0;

class Node//the node in FPtree
{
public:
	int value;//the value
	int num;//the number
	Node *parent;//parent node
	Node* Son;
	Node* nextSamePar;
	Node* nextRight;
public:
	Node()
	{
		value = 0;
		num = 0;
		Son = NULL;
		nextSamePar = NULL;
		nextRight = NULL;
	}
};

class Header//the data struct of head table
{
public:
	int value;
	double num;
	Header *parent;
	Header *son;
	Node *nextRight;//the nodes linked with the head table
	int rNum;//the number of nodes linked with the head table
public:
	Header()
	{
		value = 0;
		num = 0;
		rNum = 0;
		son = NULL;
		parent = NULL;
		nextRight = NULL;
	}
};

Header *fphead;
Node *fpTree;
double minsup;
double transNum;

class trans
{
public:
	char *str;
	trans *next;
	int len;
	int tid;
public:
	trans()
	{
		len = 0;
		tid = 0;
		next = NULL;
		str = NULL;
	}
};

trans *Dataset;//the dataset sotred in the memory
trans *strans = new trans;

void ReadData(FILE *fIn)
{
	transNum = 0;
	int i, j;
	char str[BUFFER];
	Dataset = strans;
	for (i = 0; fgets(str, BUFFER, fIn); i++)//Counting the support of every candidate itemset
	{
		if (transNum > 0)
		{
			strans->next = new trans;
			strans = strans->next;
		}
		for (j = 0; j < BUFFER && str[j] != '\0'; j++)
		{

		}
		strans->str = new char[j+1];
		strans->len = j+1;
		memcpy(strans->str, str, (j+1)*sizeof(char));
		strans->str[j] = '\0';
		strans->next = NULL;
		strans->tid = i;
		transNum++;
	}
}

Header *FreItem(double minsup)//Scanning the whole dataset once and output the headTable
{
	char * token;
	int value;
	int i;
	char str[BUFFER];
	for (strans = Dataset; strans != NULL; strans = strans->next)//Counting the support of every item
	{
		memcpy(str, strans->str, strans->len);
		token = strtok(str, " \t\n");
		while (token != NULL)
		{
			if (isdigit(*token))
			{
				value = atoi(token);
				if (value < MAXITEMNUM)
				{
					Items[value]++;
					if (value > itemsize)
					{
						itemsize=value;
					}
				}
			}
			token = strtok(NULL, " \t\n");
		}
	}

	Header *Head;
	Head = new Header;
	Head->num = -1;
	Head->parent = Head;
	Head->value = -1;
	Head->son = Head;
	Head->rNum = 0;

	for (i = 0; i < itemsize+1; i++)//building the head table
	{
		Items[i] = Items[i] + 0.00001*double(i);
		if (Items[i] >= minsup*transNum)
		{
			Header *Hsearch;
			Hsearch = Head;
			Header *Item;
			Item = new Header;
			Item->num = Items[i];
			Item->value = i;
			Item->rNum = 0;
			while(!(Item->num >= Hsearch->num && (Item->num<=Hsearch->parent->num || Hsearch->parent->num==-1)))
			{
				Hsearch = Hsearch->parent;
			}
			Item->son = Hsearch;
			Item->parent = Hsearch->parent;
			Hsearch->parent->son = Item;
			Hsearch->parent = Item;
		}
		else
		{
			Items[i] = -1;
		}
	}
	return Head;//Output the headTable
}

bool compare(int a, int b)//the operator to sort items in the transaction
{
	if (Items[a] == Items[b])
	{
		return a > b;
	}
	else
	{
		return Items[a]>Items[b];
	}
}

Node* FPtree(Header *Head)//Build FP-tree
{
	//rewind(fIn);//reset the fIn to the begin of file
	Node *Tree;//The head of fp-tree
	Tree = new Node;
	Tree->num = -1;
	Tree->value = -1;
	Tree->parent = NULL;
	//Tree->sonNum = 0;
	char * token;
	int value;
	int j,shnum;
	for (strans = Dataset; strans != NULL; strans = strans->next)
	{
		int trans[BUFFER] = {0};
		j = 0;
		token = strtok(strans->str, " \t\n");
		while (token != NULL)//read the transaction and put it into an array
		{
			if (isdigit(*token))
				value = atoi(token);
			if (value < itemsize+1)
			{
				if (Items[value] > 0)
				{
					trans[j] = value;
					j++;
				}
			}
			token = strtok(NULL, " \t\n");
		}
		sort(trans, trans + j, compare);//sort the items in the transactions according to the frequents
		Header *Hsearch;					
		Hsearch = Head->son;
		Node *Tsearch;
		Tsearch = Tree;
		for (shnum = 0; shnum < j && Items[trans[0]]>0 && Hsearch->value>0;)//Looking for items belonging to trans and Head
		{
			if (Hsearch->value == trans[shnum])//The item belonging to trans and Head
			{
				shnum++;
				int find = 0;
				Node *sSon;
				for (sSon=Tsearch->Son; sSon != NULL; sSon = sSon->nextSamePar)//search the son of current node of tree
				{
					if (sSon->value == Hsearch->value)//if there is a node whose value is the item
					{
						sSon->num++;//its number puls one
						find = 1;
						Tsearch = sSon;//the current node is changed
						break;
					}
					if (sSon->nextSamePar == NULL)
					{
						break;
					}
				}
				if (find == 0)//if there is no son of the current node whose value is the item
				{
					if (Tsearch->Son == NULL)
					{
						Tsearch->Son = new Node;
						sSon = Tsearch->Son;
					}
					else
					{
						sSon->nextSamePar = new Node;
						sSon = sSon->nextSamePar;
					}
					sSon->value = Hsearch->value;
					sSon->num = 1;
					//Tsearch->Son[m]->sonNum = 0;
					sSon->parent = Tsearch;
						
					Tsearch = sSon;//the current node is changed
					if (Hsearch->rNum == 0)
					{
						Hsearch->nextRight = Tsearch;
					}
					else
					{
						Tsearch->nextRight = Hsearch->nextRight;
						Hsearch->nextRight = Tsearch;
					}
					Hsearch->rNum++;
				}
			}
			Hsearch = Hsearch->son;
		}
	}
	return Tree;
}

double sItems[MAXITEMNUM] = { 0 };

Header *condFreItem(Header *sH, double minsup)
{
	int i,Num;
	memset(sItems, 0, sizeof(double)*MAXITEMNUM);
	Node *sNode;
	for (sNode = sH->nextRight; sNode!=NULL; sNode = sNode->nextRight)//sH->Right[0],sH->Right[1]....are the conditional basis
	{
		Node *sC;
		sC = sNode->parent;
		Num = sNode->num;
		while (sC->value != -1)
		{
			sItems[sC->value] = sItems[sC->value] + Num;
			sC = sC->parent;
		}
	}

	Header *sHead = new Header;
	sHead->num = -1;
	sHead->parent = sHead;
	sHead->value = -1;
	sHead->son = sHead;
	sHead->rNum = 0;

	for (i = 0; i < itemsize+1; i++)//building the conditional head table
	{
		sItems[i] = sItems[i] + 0.00001*double(i);
		if (sItems[i] >= minsup*transNum)
		{
			Header *Hsearch;
			Hsearch = sHead;
			Header *Item;
			Item = new Header;
			Item->num = sItems[i];
			Item->value = i;
			Item->rNum = 0;
			while (!(Item->num >= Hsearch->num && (Item->num <= Hsearch->parent->num || Hsearch->parent->num == -1)))
			{
				Hsearch = Hsearch->parent;
			}
			Item->son = Hsearch;
			Item->parent = Hsearch->parent;
			Hsearch->parent->son = Item;
			Hsearch->parent = Item;
		}
		else
		{
			sItems[i] = -1;
		}
	}
	return sHead;
}

bool comparecond(int a, int b)//the operator to sort items in the transaction
{
	if (sItems[a] == sItems[b])
	{
		return a > b;
	}
	else
	{
		return sItems[a]>sItems[b];
	}
}

Node* condFPtree(Header *sH, Header *sHead)	//building the conditional fp tree
{
	Node *sTree;
	sTree = new Node;
	sTree->num = -1;
	sTree->value = -1;
	sTree->parent = NULL;
	//sTree->sonNum = 0;
	int j, shnum;
	Node *sNode;
	for (sNode = sH->nextRight; sNode!=NULL; sNode = sNode->nextRight)
	{
		Node *sC;
		sC = sNode->parent;
		int trans[BUFFER] = { 0 };
		j = 0;
		while (sC->value != -1)	//store the transaction of conditional basis
		{
			trans[j] = sC->value;
			sC = sC->parent;
			j++;
		}
		sort(trans, trans + j, comparecond);//sort the items in the trans of the conditional basis according to their support in the conditional basis
		Header *Hsearch;
		Hsearch = sHead->son;
		Node *Tsearch;
		Tsearch = sTree;
		for (shnum = 0; shnum < j && sItems[trans[0]]>0 && Hsearch->value>0;)//Looking for items belonging to trans and Head
		{
			if (Hsearch->value == trans[shnum])//The item belonging to trans and Head
			{
				shnum++;
				int find = 0;
				Node *sSon;
				for (sSon = Tsearch->Son; sSon!=NULL; sSon = sSon->nextSamePar)//search the son of current node of tree
				{
					if (sSon->value == Hsearch->value)//if there is a node whose value is the item
					{
						sSon->num= sSon->num+ sNode->num;//its number puls one
						find = 1;
						Tsearch = sSon;//the current node is changed
						break;
					}
					if (sSon->nextSamePar == NULL)
					{
						break;
					}
				}
				if (find == 0)//if there is no son of the current node whose value is the item
				{
					if (Tsearch->Son == NULL)
					{
						Tsearch->Son = new Node;
						sSon = Tsearch->Son;
					}
					else
					{
						sSon->nextSamePar = new Node;
						sSon = sSon->nextSamePar;
					}
					sSon->value = Hsearch->value;
					sSon->num = sNode->num;
					//Tsearch->Son[m]->sonNum = 0;
					sSon->parent = Tsearch;

					Tsearch = sSon;//the current node is changed

					if (Hsearch->rNum == 0)
					{
						Hsearch->nextRight = Tsearch;
					}
					else
					{
						Tsearch->nextRight = Hsearch->nextRight;
						Hsearch->nextRight = Tsearch;
					}
					Hsearch->rNum++;
				}
			}
			Hsearch = Hsearch->son;
		}
	}
	return sTree;
}

void FPgrowth(Node *Tree, Header *Head, double minsup, int itemset[BUFFER],int len)	//mining the frequent itemsets
{
	Header *sH;
	sH = Head->parent;
	int i;
	while (sH->value != -1)
	{
		int beta[BUFFER] = { 0 };//show the frequent itemsets
		int length = len;
		for (i = 0; i < length; i++)
		{
			beta[i] = itemset[i];
		}
		beta[length] = sH->value;
		length++;
		for (i = 0; i < length; i++)
		{
			printf("%d ", beta[i]);
		}
		printf("support %d\n", long(sH->num)); patternNum++;
		Header *sHead;
		sHead = condFreItem(sH, minsup);//building the headtable of the conditional basis
		Node *sTree;
		sTree = condFPtree(sH, sHead);//buildng the conditional fp tree
		FPgrowth(sTree, sHead, minsup, beta, length);//frquent itemsets are growthing
		sH = sH->parent;
	}
}

int main(int argc, char** argv)
{
	FILE *fIn;
	if (argc != 3)
	{
		printf("Usage: fpgrowth <input-filename> <minimum-support>\n");
	}
	else if (!(fIn = fopen(argv[1], "r")))
	{
		printf("Error in input file\n");
	}
	else if (atof(argv[2]) < 0)
	{
		printf("invalid minimum support");
	}
	else
	{
		minsup = atof(argv[2]);//setting the minimum support
		ReadData(fIn);//put the data into the memory
		fphead = FreItem(minsup);//scanning the dataset and find those items with supports higher than minsup
		fpTree = FPtree(fphead);//building fp-tree
		int itemset[BUFFER] = { 0 };
		int len = 0;
		FPgrowth(fpTree, fphead, minsup, itemset, len);//mining the freqeunt itemsets
		printf("\n%d patterns are output\n", patternNum);
	}
	printf("\npush return to exit\n");
	getchar();
	return 0;
}
