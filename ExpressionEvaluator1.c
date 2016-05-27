#include<stdio.h>
#include<conio.h>
#include<stdlib.h>
#pragma warning(disable:4001)

struct valStack{
	//single linked stack structure for operands
	long double val;
	struct valStack* preVal;
};

struct operatorStack{
	//single linked stack structure for operators
	char op;
	struct operatorStack* prevOp;
};

long double takeVal(FILE *fp, char *ch ,FILE *fp1 ){
	//takes the operand value
	long double operand=0;
	char isDecimal='n';
	double dec=0.1;
	if (*ch<'0' || *ch>'9')		//invalid number error
		return-1;
	while((*ch>='0' && *ch<='9') || *ch == '.'){	
		if(*ch == '.')			//checks for decimal
			isDecimal='y';
		else if(isDecimal == 'n')		//if not deciml
			operand= operand*10 + ((int)*ch)-'0';
		else{							//adds decimal value
			operand= operand + (dec* (((int)*ch)-'0'));
			dec=dec*0.1;
		}
		*ch=(char)fgetc(fp);
		if(*ch!='\n' && *ch!= EOF)	
			fputc(*ch,fp1);
	}
	return operand;
}

void pushVal(long double operand,struct valStack*valHead){
	//pushes operand on top 
	struct valStack* newVal=(struct valStack*) malloc(sizeof(struct valStack));
	newVal->val=operand;
	newVal->preVal=valHead->preVal;
	valHead->preVal=newVal;
}

long double pow(long double base,long int exp){
	//calculates base power exp
	long double retVal=1;
	while(exp){
		if (exp & 1)
			retVal= retVal * base;
		exp>>=1;
		base= base *base;
	}
	return retVal;
}

long double popVal(struct valStack* valHead){
	//struct valStack* tempDel = (struct valStack*) malloc(sizeof(struct valStack*));
	struct valStack* temp= valHead->preVal;
	long double ret=valHead->preVal->val;
	valHead->preVal=valHead->preVal->preVal;
	free(temp);
	temp=NULL;
	return ret;
}

void open( FILE *fp,char *str, char* ch){
	if (fopen_s(&fp,str,ch)!=0){
		//error opening file
		printf("Can not open file tempInput.txt");
		_getch();
		exit(0);
	}
}

void error(char *str,FILE *fp,FILE *fp1){
	//prints error message for that line
	char ch=(char)fgetc(fp);
	while(ch!='\n' && ch!=EOF){	//goto end of line and print
		fputc(ch,fp1);
		ch=(char)fgetc(fp);
	}
	printf_s("%s\n",str);
	fprintf_s(fp1," %s\n",str);
}

void compute(struct operatorStack* operatorHead,struct valStack* valHead,FILE *fp,FILE *fp1){
	//computes top 2 operands in valStack with top operator
	long double op1,op2;
	struct operatorStack* presentOp=operatorHead->prevOp;
	char ch=presentOp->op;
	op2=popVal(valHead);	//first operand
	op1=popVal(valHead);	//second operand
	operatorHead->prevOp=operatorHead->prevOp->prevOp;
	free(presentOp);
	switch(ch){
		case '+':
			pushVal((op1+op2),valHead);
			return;
		case '-':
			pushVal((op1-op2),valHead);
			return;
		case '/':
			if(op2 == 0)

				error("undified: divide by zero",fp,fp1);
			else
				pushVal(((op1*1.0)/(op2*1.0)),valHead);
			return;
		case '*':
			pushVal((op1*op2),valHead);
			return;
		case '^':
			pushVal(pow(op1,op2),valHead);
	}
}

int priority(char op){
	//prioratize operators
	switch(op){
					case '+':
					case '-':
						return 0;
					case '/':
					case '*':
						return 1;
					case '^':
						return 2;
					default:
						return -1;
	}
}

void pushOp(char op,struct operatorStack* operatorHead,struct valStack*valHead,FILE *fp,FILE *fp1){
	//computes if more precedence operator present in stack then pushes operator on top 
	int precidence=priority(op);
	struct operatorStack *newOp=(struct operatorStack *) malloc(sizeof(struct operatorStack));
	while ((operatorHead->prevOp != NULL) && (priority(operatorHead->prevOp->op) >= precidence)){
		compute(operatorHead,valHead,fp,fp1);
	}
	newOp->op=op;
	newOp->prevOp=operatorHead->prevOp;
	operatorHead->prevOp=newOp;
}

void initialize(struct operatorStack* operatorHead,struct valStack* valHead,char isOperator){
	//initializes for new expression
	isOperator='n';
	operatorHead->prevOp=NULL;
	valHead->preVal=NULL;
	valHead->val=0;
}

int main(){
	FILE *fp,*fp1;
	struct valStack* valHead=(struct valStack*) malloc(sizeof(struct valStack));
	struct operatorStack* operatorHead=(struct operatorStack*) malloc(sizeof(struct operatorStack));
	long double operand;
	char ch=' ',isOperator='n';

	if (fopen_s(&fp,"input.txt","r")!=0){
		//error opening file
		printf("Can not open file input.txt");
		_getch();
		return 1;
	}
	if (fopen_s(&fp1,"tempInput.txt","w")!=0){
		//error opening file
		printf("Can not open file tempInput.txt");
		_getch();
		return 1;
	}
	
	while((ch=(char)fgetc(fp))!=EOF){
		fputc(ch,fp1);
		initialize(operatorHead,valHead,isOperator);
		while(ch!='\n' && ch!=EOF ){
			//removes extra spaces
			while(ch==' ' || ch=='\t'){
				ch=(char)fgetc(fp);
				if(ch!='\n')
					fputc(ch,fp1);
			}
			if(ch=='\n')
				break;
			//for operand
			if(isOperator=='n'){	
				operand=takeVal(fp,&ch,fp1);
				if (operand==-1){
					error("syntax error",fp,fp1);
					initialize(operatorHead,valHead,isOperator);
					ch=(char)fgetc(fp);
					fputc(ch,fp1);
					continue;
				}
				pushVal(operand,valHead);
				isOperator='y';
			}
			else{		
				//for operator
				if(priority(ch)==-1){
					error("invalid operator",fp,fp1);
					initialize(operatorHead,valHead,isOperator);
					ch=(char)fgetc(fp);
					fputc(ch,fp1);
					continue;
				}	
				if(ch=='*'){
					ch=(char)fgetc(fp);
					fputc(ch,fp1);
					//checking for power operator
					if(ch=='*'){
						pushOp('^',operatorHead,valHead,fp,fp1);
						ch=(char)fgetc(fp);		//take next char
						fputc(ch,fp1);
					}
					else
						pushOp('*',operatorHead,valHead,fp,fp1);
				}
				else{
					//for other operators
					pushOp(ch,operatorHead,valHead,fp,fp1);
					ch=(char)fgetc(fp);
					fputc(ch,fp1);
				}
				isOperator='n';
			}
		}
		if(isOperator=='n' && valHead->preVal!=NULL){	
			//checks if ending with operator and raises error
			error("syntax error",fp,fp1);
			continue;
		}
		while(operatorHead->prevOp !=NULL){
			ungetc(ch,fp);
			compute(operatorHead,valHead,fp,fp1);
			if(ch != '\n')		//go front again
				ch=fgetc(fp);
		}
		if(valHead->preVal!=NULL && isOperator!='e'){
			printf( "%g \n",valHead->preVal->val);
			fprintf_s(fp1,"=%g",valHead->preVal->val);
		}
		isOperator='n';			//initializes to start from operand
	}
	//goes back to starting position and reads 
	fclose(fp);
	fclose(fp1);
    	remove("input.txt");
	rename("tempInput.txt","input.txt");	//it will make it look like editing the same file
	_getch();
	return 0;
}
