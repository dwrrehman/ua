#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <iso646.h>
#include <unistd.h>

#define red	"\x1B[31m"
#define green	"\x1B[32m"
#define yellow	"\x1B[33m"
#define blue	"\x1B[34m"
#define magenta	"\x1B[35m"
#define cyan   	"\x1B[36m"
#define reset	"\x1B[0m"

typedef int64_t nat;

static const nat delay = 999;

static const nat debug = 1;


struct node {
	struct node* left;
	struct node* right;
	nat data;
};

struct entry {
	struct node** this;
	nat branch;
};


static nat node_count = 1, stack_count = 0;
static nat history_count = 0;
static struct node** history = NULL;


static void print_tree(struct node* root) {
	int count = 0;
	struct node stack[128] = {0};
	if (root) stack[count++].left = root;
	else printf(yellow "{empty}\n" reset); 

	while (count) {
		struct node* top = stack + (count - 1);
		struct node* node = top->left;

		if (top->data == 0) {
			printf(cyan "%p " reset "{\n", (void*) node);
			for (int i = 0; i < count; i++) printf(blue "│   " reset);
			printf("." green "left " reset "= ");

			stack[count - 1].data++;
			if (node->left) {
				stack[count].data = 0;
				stack[count++].left = node->left;
				continue;
			} else printf(yellow "{null}\n" reset); 

		} else if (top->data == 1) {
			for (int i = 0; i < count; i++) printf(blue "│   " reset);
			printf("." red "right " reset "= ");

			stack[count - 1].data++;
			if (node->right) { 
				stack[count].data = 0;
				stack[count++].left = node->right; 
				continue; 
			} else printf(yellow "{null}\n" reset); 

		} else {
			count--;
			for (int i = 0; i < count; i++) printf(blue "│   " reset);
			printf("} \n");
		}
	}
}

static void print_entry(struct entry e) {
	printf("struct entry { .this = %p, .branch = %llu }\n", (void*) e.this, e.branch);
	if (e.this) printf("---> [ *(.this) = %p ] \n", (void*) *e.this);
}


static bool equals(struct node* a, struct node* b) {
	if (not a and not b) return true;
	if (a and not b) return false;
	if (not a and b) return false;
	if (a and b) {
		if (equals(a->left, b->left) and equals(a->right, b->right)) return true;
	}
	return false;
}

static bool found(struct node* tree) {
	for (nat i = 0; i < history_count; i++) {
		if (equals(history[i], tree)) return true;
	}
	return false;
}

static struct node* duplicate(struct node* tree) {
	if (not tree) return NULL;
	struct node* n = calloc(1, sizeof(struct node));
	n->data = tree->data;
	n->left = duplicate(tree->left);
	n->right = duplicate(tree->right);
	return n;
}

static void print_history(void) {
	puts("[begin-history]");
	for (nat i = 0; i < history_count; i++) {
		printf("printing history element [%llu]...\n", i);
		print_tree(history[i]);
		puts("");
	}
	puts("[end-history]");
}

static void print(struct node* root) {
	if (found(root)) {
		if (debug) printf( magenta "found a duplicate.. skipping...\n" reset);
		return;
	}

	printf(green "--------------------[history_index = %llu] [node_count = %llu] [stack_count = %llu]---------------------\n" reset, 
					history_count, node_count, stack_count);

	if (debug) printf("printing binary tree with [history_index=%llu]...\n", history_count);
	print_tree(root);
	
	history = realloc(history, sizeof(struct node*) * (size_t) (history_count + 1));
	history[history_count++] = duplicate(root);

	//print_history();

	if (delay != 999) sleep(delay); else getchar();
}

int main(int argc, const char** argv) {
	if (argc != 1 and argc != 2) return puts("supply a max_node_count! (default 5)");
	const nat max_node_count = argc > 1 ? atoi(argv[1]): 5;

	struct node* root = calloc(1, sizeof(struct node));


	struct node* example = calloc(1, sizeof(struct node));
	example->left = calloc(1, sizeof(struct node));
	example->left->left = calloc(1, sizeof(struct node));

	example->right = calloc(1, sizeof(struct node));
	example->right->right = calloc(1, sizeof(struct node));


	struct node* bubbles = calloc(1, sizeof(struct node));
	bubbles->left = calloc(1, sizeof(struct node));
	bubbles->left->right = calloc(1, sizeof(struct node));

	bubbles->right = calloc(1, sizeof(struct node));
	bubbles->right->left = calloc(1, sizeof(struct node));


	struct node* pasta = calloc(1, sizeof(struct node));
	pasta->left = calloc(1, sizeof(struct node));
	pasta->left->right = calloc(1, sizeof(struct node));

	pasta->right = calloc(1, sizeof(struct node));
	pasta->right->right = calloc(1, sizeof(struct node));


	struct node* train = calloc(1, sizeof(struct node));
	train->left = calloc(1, sizeof(struct node));
	train->left->left = calloc(1, sizeof(struct node));

	train->right = calloc(1, sizeof(struct node));
	train->right->left = calloc(1, sizeof(struct node));








	struct entry stack[128] = {0};
	stack[stack_count].branch = 0;
	stack[stack_count++].this = &root;
top:;
	struct entry* top = stack + stack_count - 1;
	const nat b = top->branch;

	if (debug) printf("TOS:(@ %llu)  ", stack_count - 1);
	if (debug) print_entry(*top);

	if (b == 0) { 
		if (debug) puts("tos : 0 0");

		if (node_count == max_node_count) 
			print(root);

		(*(top->this))->left = NULL;
		(*(top->this))->right = NULL;

		top->branch++;
		goto top;
	}

	else if (b == 1) {
		if (debug) puts("tos : 0 1");

		if (node_count >= max_node_count) { if (debug) puts("AT MAX NODES..."); goto pop; }	

		(*(top->this))->left = NULL;
		(*(top->this))->right = calloc(1, sizeof(struct node));
		stack[stack_count++] = (struct entry) {.this = &(*(top->this))->right, .branch = 0 };

		node_count++;
		top->branch++;
		goto top;
	}

	else if (b == 2) {
		if (debug) puts("tos : 1 0");

		(*(top->this))->left  = calloc(1, sizeof(struct node));
		stack[stack_count++] = (struct entry) {.this = &(*(top->this))->left, .branch = 0 };
		(*(top->this))->right = NULL;

		top->branch++;
		goto top;
	}

	else if (b == 3) { 
		if (debug) puts("tos : 1 1");
	
		if (node_count >= max_node_count) { if (debug) puts("AT MAX NODES..."); goto pop; }

		(*(top->this))->left  = calloc(1, sizeof(struct node));
		stack[stack_count++] = (struct entry) {.this = &(*(top->this))->left, .branch = 0 };
		(*(top->this))->right = calloc(1, sizeof(struct node));
		stack[stack_count++] = (struct entry) {.this = &(*(top->this))->right, .branch = 0 };

		node_count++;
		top->branch++;
		goto top;
	}

	else if (b == 4) {
		if (debug) puts("tos : (4)");

	pop:	if (debug) puts("popping...");

		if (b == 0) {}
		if (b == 1) {}
		if (b == 2) {}
		if (b == 3) node_count--;
		if (b == 4) node_count -= 2;
		
		(*(top->this))->left = NULL;
		(*(top->this))->right = NULL;

		stack_count--;
		if (stack_count) goto top;
	} 
	else abort();

	if (max_node_count == 5) {
		if (not found(example)) { printf("could not find example: \n"); print(example); }
		if (not found(bubbles)) { printf("could not find bubbles: \n"); print(bubbles); }
		if (not found(pasta)) { printf("could not find pasta: \n"); print(pasta); }
		if (not found(train)) { printf("could not find train: \n"); print(train); }
	}	
}














































		









/*



		//stack[stack_count++] = (struct entry) {.this = &(*(top->this))->left, .branch = 0 };
		//stack[stack_count++] = (struct entry) {.this = &(*(top->this))->right, .branch = 0 };






printf(green "--------------------[node_count = %llu] [stack_count = %llu]---------------------\n" reset, node_count, stack_count);
		if (debug) puts("printing binary tree...");
		print(root);
		if (delay != 999) sleep(delay); else getchar();
	
		top->branch++;
		*(top->this) = calloc(1, sizeof(struct node));
		stack[stack_count++] = (struct entry) {.this = &(*(top->this))->right, .branch = 0 };
		goto top;





if (node_count >= max_node_count) { if (debug) puts("AT MAX NODES..."); goto pop; }

		if (debug) puts("tos has 2 branch.");
	
	
		printf(green "--------------------[node_count = %llu] [stack_count = %llu]---------------------\n" reset, node_count, stack_count);
		if (debug) puts("printing binary tree...");
		print(root);
		if (delay != 999) sleep(delay); else getchar();
	

		top->branch++;
		node_count++;
		stack[stack_count++] = (struct entry) {.this = &(*(top->this))->left, .branch = 0 };
		stack[stack_count++] = (struct entry) {.this = &(*(top->this))->right, .branch = 0 };
		goto top;








static int count_nodes(struct node* root) {

	int count = 0, total_count = 0;
	struct node* stack[128] = {0};

	if (root) stack[count++] = root;

	while (count) {
		struct node* top = stack[--count];
		if (top->left)  stack[count++] = top->left;
		if (top->right) stack[count++] = top->right;
		total_count++;
	}

	return total_count;
}









//puts("checking for pop...");
			//if (count_nodes(root) >= max_node_count) {
			//}




	
		if (not *top) {

			if (count_nodes(root) >= max_node_count) { }

			*top = calloc(1, sizeof(struct node));

		}
		














2306117.214736



	so we first breakdown the problem to be the following possibilities for each node extension:


		0 : no new nodes added.    [leaf node]

		1 : just a left node added.    head takes the left node    by force. 

		2 : just a right node is added     head takes the right node      by force. 

		3 : both a left node and a right node is added, 			and head takes the left node

		4 : both a left node and a right node is added,   			and head takes the right node 

		5 : both a left node and a right node is added,   			and both left and right is extended...?






no, this doesnt make sense. 


		i feel like we need to make the       0-1-2-3   decision     on both children, if they exist.  if they don't, then thats a problem for the prior decision to solve. 




	so revising things, we get:


	
		0 : no new nodes added.    [leaf node]

		1 : just a left node added.    

		2 : just a right node is added     

		3 : both a left node and a right node is added, 
		


	we still need to enumerate these possibilties for each node. 


	but now, the trick is.... we now need to 

			first make the decision 





		okay okay 



				so what if we enumerated      ie    put in an ordering     used to help us go through possibilities 


					what if we enumerated the left and right, in terms of pushing them onto the stack!?



							ie, we push the left on stack,   (ie, it has a lower   deeper    effective on the possibility space
											)



							and then we push the right on the stack

								which means it is quicker    and faster to change   usually 


								okay, sure     this kinda seems required 




							

								oh wait, when we change the node's       value        (ie,  0,1,2,3)


									that causes us to push some things onto the stack,	up to two 

										based on what value we change it to. yay. 

											cool that sounds good ithink. 


									yay


										



*/






















/*
	we need to loop over the stack, that we are making, each time we change a node in the stack?

		ie, its a counting algorithm, like the nfa

		hmm



		we also need to loop through the 4 possibilities for every nullptr node we see:      0 branches,  left branch, right branch, both branches.


		but, only go from 0 to 1    and 2 to 3     if the node count allows us to.




				





 also we should print a box character around every binary tree!!!


very cool



*/













/*
	root = 			calloc(1, sizeof(struct node));
	root->left = 		calloc(1, sizeof(struct node));
	root->left->left = 	calloc(1, sizeof(struct node));
	root->right = 		calloc(1, sizeof(struct node));
*/

