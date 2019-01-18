#include <stdio.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h> 
#include <sys/wait.h>
#include <sys/stat.h>
#include <readline/readline.h>
#include <readline/history.h>

#define BUFSIZE 256
#define MAXARGS 20

// define the type of command
#define EXEC 1
#define REDIR 2
#define PIPE 3
#define LIST 4
#define BACK 5
#define NOTBACK 0 // for building the parse tree

struct cmd {
	int type;
};

struct execcmd {
	int type;
	int argc;
	char *argv[MAXARGS];
};

struct redircmd {
	int type;
	struct cmd *cmd;
	char *in_file;
	char *out_file;
};

struct pipecmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct listcmd {
  int type;
  struct cmd *left;
  struct cmd *right;
};

struct backcmd {
	int type;
	struct cmd *cmd;
};

// for building parse tree
struct node;
struct node {
	int type;
	int argc;
	char argv[MAXARGS][MAXARGS];
	struct node* parent;
	struct node* child;
	struct node* left;
	struct node* right;
};

const char whitespace[] = " \t\r\n\v";
const char symbols[] = "<|>&;";
struct cmd *cmds[100]; // to free allocated resources
struct node *nodes[100];
int nodes_ct = 0;
int cmds_ct = 0;

sigjmp_buf ctrlc_buf;
int child_pid_back = -2;
int parent_pid;
int child_pid;

void
update_nodes(struct node *node) {
	nodes[nodes_ct] = node;
	nodes_ct++;
}

void
free_nodes() {
	for (int i = 0; i < nodes_ct; i++) {
		free(nodes[i]);
	}
	nodes_ct = 0;
}

void
update_cmds(struct cmd *cmd) {
	cmds[cmds_ct] = cmd;
	cmds_ct++;
}

void
free_cmds() {
	for (int i = 0; i < cmds_ct; i++) {
		free(cmds[i]);
	}
	cmds_ct = 0;
}

struct node*
make_node() {
	struct node *node;
	node = malloc(sizeof(*node));
	memset(node, 0, sizeof(*node));
	update_nodes(node);
	return node;
}

struct cmd*
make_execcmd(void) {
	struct execcmd *cmd;
	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd -> type = EXEC;
	update_cmds((struct cmd*) cmd);
	return (struct cmd*) cmd;
}

struct cmd*
make_redircmd(struct cmd *subcmd, char *in_file, char *out_file) {
	struct redircmd *cmd;
	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd -> type = REDIR;
	cmd -> cmd = subcmd;
	cmd -> in_file = in_file;
	cmd -> out_file = out_file;
	update_cmds((struct cmd*) cmd);
	return (struct cmd*) cmd;
}

struct cmd*
make_pipecmd(struct cmd *left, struct cmd *right) {
	struct pipecmd *cmd;
	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd -> type = PIPE;
	cmd -> left = left;
	cmd -> right = right;
	update_cmds((struct cmd*) cmd);
	return (struct cmd*) cmd;
}

struct cmd*
make_listcmd(struct cmd *left, struct cmd *right) {
	struct listcmd *cmd;
	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd -> type = LIST;
	cmd -> left = left;
	cmd -> right = right;
	update_cmds((struct cmd*) cmd);
	return (struct cmd*) cmd;
}

struct cmd*
make_backcmd(struct cmd *subcmd) {
	struct backcmd *cmd;
	cmd = malloc(sizeof(*cmd));
	memset(cmd, 0, sizeof(*cmd));
	cmd -> type = BACK;
	cmd -> cmd = subcmd;
	update_cmds((struct cmd*) cmd);
	return (struct cmd*) cmd;
}

/*
get user input
*/
void 
get_line(char *line) {
	printf("shell> ");
	memset(line, 0, BUFSIZE);
	fgets(line, BUFSIZE, stdin);
}

/*
shift string right by one position
*/
void
shift_right(char *line, int ps) {
	int i = 0;
	char temp_char;
	char prev_char = line[ps];
	line[ps] = ' ';
	while (prev_char != '\0') {
		temp_char = line[ps];
		line[ps] = prev_char;
		prev_char = temp_char;
		ps++;
	}
}

void 
split_line(char *line, char *eline, char argv[][MAXARGS], int *argc){
	char *word;
	char *temp;

	// add space between special symbols
	int ps = 0;
	while (line[ps] != '\0') {
		if (strchr(symbols, line[ps])) {
			shift_right(line, ps);
			shift_right(line, ps);
			line[ps+1] = line[ps];
			line[ps] = ' ';
			line[ps+2] = ' ';
			ps++;
		}
		ps ++;
	}

	// tokenize line
	word = strtok(line, whitespace);
	int i = 0;
	while (word != NULL) {
		temp = strdup(word);
		strcpy(argv[i], temp);
		free(temp);

		word = strtok(NULL, whitespace);
		i++;
	}
	*argc = i;
}

void
find_pos(struct node *node, struct execcmd *cmd, int pos[], int *count, char symbol) {
	// printf("print in find_pos()\n");
	char symbol_str[] = {symbol, '\0'};

	// clear buffer
	*count = 0;
	memset(pos, 0, 10);
	// find positions of the symbol
	if (node != NULL) { // search in the node
		for (int i = 0; i < node->argc; i++) {
			if (strcmp(node->argv[i], symbol_str) == 0) {
				pos[*count] = i;
				*count = *count + 1;
			}
		}
	}
	else { // search in the cmd
		for (int i = 0; i < cmd->argc; i++) {
			if (strcmp(cmd->argv[i], symbol_str) == 0) {
				pos[*count] = i;
				*count = *count + 1;
			}
		}
	}
}

/*
copy argv[start] ... argv[end-1] to node->argv
*/
void
copy_argv(struct node *node, struct cmd *cmd, char argv[][MAXARGS], int start, int end) {
	// printf("print in copy_argv()\n");

	int i;
	if (node != NULL && cmd != NULL) {
		perror("error: copy_argv()\n");
		exit(0);
	}

	if (node != NULL) {
		// CASE 1: 1 argv
		if (start == end) {
			strcpy(node -> argv[0], argv[start]);
			node -> argc = 1;
		}
		else
		{
			// CASE 2: > 1 argv
			int length = end - start;
			for (i = 0; i < length; i++) {
				strcpy(node -> argv[i], argv[start+i]);
			}
			node -> argc = length;
		}
	}
	if (cmd != NULL) {
		struct execcmd *execcmd = (struct execcmd *)cmd;

		if (start == end) {
			execcmd -> argv[0] = argv[start];
			execcmd -> argc = 1;
			execcmd -> argv[1] = NULL; // null terminate args
		}
		else
		{
			for (i = 0; i < end - start; i++) {
				execcmd -> argv[i] = argv[start+i];
			}
			execcmd -> argc = end - start;
			execcmd -> argv[i] = NULL; // null terminate args
		}
	}
}

void
init_node(struct node* node, int type, struct node* parent,
	struct node* child, struct node* left, struct node* right) {
	node -> type = type;
	node -> parent = parent;
	node -> child = child;
	node -> left = left;
	node -> right = right;
}

int
check_redir(struct cmd *cmd) {
	struct execcmd *execcmd = (struct execcmd *)cmd;

	char in_redir[] = {'<', '\0'};
	char out_redir[] = {'>', '\0'};
	for (int i = 0; i < execcmd -> argc; i++) {
		if (strcmp(execcmd -> argv[i], in_redir) == 0 ||
			strcmp(execcmd -> argv[i], out_redir) == 0) {
			return 1;
		}
	}
	return 0;
}

void
parse_redir(struct cmd *cmd, char **in_file, char **out_file) {
	// printf("print in parse_redir()\n");
	struct execcmd *execcmd = (struct execcmd *) cmd;

	int inredir_ct;
	int outredir_ct;
	int inredir_pos[10];
	int outredir_pos[10];

	find_pos(NULL, execcmd, inredir_pos, &inredir_ct, '<');
	find_pos(NULL, execcmd, outredir_pos, &outredir_ct, '>');

	if (inredir_ct + outredir_ct > 2) {
		perror("error: parse_redir()");
		exit(0);
	}

	if (inredir_ct == 1) {
		*in_file = execcmd->argv[inredir_pos[0] + 1];
	}

	if (outredir_ct == 1) {
		*out_file = execcmd->argv[outredir_pos[0] + 1];
	}

	// trim argv using short circuit
	int start = (inredir_ct == 1) ? inredir_pos[0] : outredir_pos[0];
	int length = execcmd -> argc;
	for (int i = start; i < length; i++) {
		execcmd -> argv[i] = NULL;
		execcmd -> argc--;
	}
}

struct cmd*
build_cmd_chain(struct node *root) {
	// printf("print in build_cmd_chain()\n");
	struct cmd *ret_cmd;

	// for building cmd chain
	struct cmd *cmd_in;
	struct cmd *cmd_out;
	struct cmd *cmd_pipe_start = NULL; // mark the start of pipecmd
	struct cmd *cmd_list_start = NULL; // mark the start of listcmd
	struct cmd *cmd_pipe_prev = NULL;
	struct cmd *cmd_list_prev = NULL;
	struct cmd *cmd_pipe_left = NULL; // for inner loop - parse pipe
	struct cmd *cmd_list_left = NULL; // for outer loop - parse list

	// for traversing parse tree
	struct node *node = root;
	struct node *node_in;

	node = node -> child;

	do {

		node_in = node -> child;		
		do {
			cmd_in = (struct cmd*) make_execcmd();

			copy_argv(NULL, cmd_in, node_in->argv, 0, node_in-> argc);
			// check redir here
			if (check_redir(cmd_in)) {
				char *in_file = NULL;
				char *out_file = NULL;
				parse_redir(cmd_in, &in_file, &out_file);
				cmd_in = make_redircmd(cmd_in, in_file, out_file);
			}

			// for building the cmd chain
			if (cmd_pipe_left == NULL) cmd_pipe_left = cmd_in; // cmd_pipe_left: EXEC
			else {
				// building pipecmd

				cmd_in = make_pipecmd(cmd_pipe_left, cmd_in);
				cmd_pipe_left = cmd_in; // cmd_pipe_left: PIPE

				// connect PIPE cmds
				if (cmd_pipe_prev == NULL) { // previous PIPE cmd
					cmd_pipe_prev = cmd_in;
				}
				else {
					((struct pipecmd *) cmd_in) -> left = ((struct pipecmd *) cmd_pipe_prev) -> right; // change curr PIPE cmd
					((struct pipecmd *) cmd_pipe_prev) -> right = cmd_in; // change prev PIPE cmd

					cmd_pipe_prev = cmd_in;
				}
				

				// if this is the start of the pipe
				if (cmd_pipe_start == NULL) 
					cmd_pipe_start = cmd_in;
			}
			// for traversing the parse tree
			node_in = node_in -> right;
		} while (node_in != NULL);
		


		// for building the cmd chain
		// update cmd_out
		cmd_out = (cmd_pipe_start == NULL) ? cmd_in : cmd_pipe_start;

		if (cmd_list_left == NULL) cmd_list_left = cmd_out;
		else {
			cmd_out = make_listcmd(cmd_list_left, cmd_out);
			cmd_list_left = cmd_out; // cmd_list_left

			if (cmd_list_prev == NULL) {
				cmd_list_prev = cmd_out;
			}
			else {
				((struct listcmd *) cmd_out) -> left = ((struct listcmd *) cmd_list_prev) -> right;
				((struct listcmd *) cmd_list_prev) -> right = cmd_out;
				cmd_list_prev = cmd_out;
			}

			if (cmd_list_start == NULL)
				cmd_list_start = cmd_out;
		}

		// reset pointers in inner loop
		cmd_pipe_prev == NULL;
		cmd_pipe_left = NULL;
		cmd_pipe_start = NULL;

		// for traversing the parse tree
		node = node -> right;
	} while (node != NULL);


	ret_cmd = (cmd_list_start == NULL) ? cmd_out : cmd_list_start;

	if (root -> type == BACK) {
		if (cmd_list_start != NULL) {
			struct cmd *temp_cmd = make_backcmd(cmd_list_left);
			((struct listcmd *)cmd_list_left) -> right = temp_cmd;
		}
		else {
			ret_cmd = make_backcmd(ret_cmd);
		}
	}

	return ret_cmd;
}

struct cmd*
build_parse_tree(char argv[][MAXARGS], int argc) {
	// printf("print in build_parse_tree()\n");
	int pipe_ct;
	int list_ct;
	int pipe_pos[10];
	int list_pos[10];
	
	// to build parse tree
	int list_prev_pos = 0;
	int pipe_prev_pos = 0;
	struct node *node; // temp node for outer loop
	struct node *node_in; // temp node for inner loop
	struct node *root;
	struct node *list_prev_node;
	struct node *pipe_prev_node;

	struct node *nodes[100]; // to free node
	int nodes_ct = 0;

	// build root node
	node = make_node();
	copy_argv(node, NULL, argv, 0, argc);
	root = node;

	// check backcmd here
	int type = NOTBACK;
	char symbol_str[] = {'&', '\0'};
	if (strcmp(argv[argc-1], symbol_str) == 0) {
		type = BACK;
		argc --;
	}
	find_pos(root, NULL, list_pos, &list_ct, ';');
	init_node(node, type, NULL, NULL, NULL, NULL);

	list_prev_node = NULL;

	// STEP-1: split line by ';'
	find_pos(root, NULL, list_pos, &list_ct, ';');
	int i = 0;
	do {
		// create a new node
		node = make_node();

		if (list_ct == 0) {
			copy_argv(node, NULL, argv, 0, argc);
		}
		// the first n-1 list
		else if (i < list_ct) {
			// copy argument to the new node
			copy_argv(node, NULL, argv, list_prev_pos, list_pos[i]);
			list_prev_pos = list_pos[i] + 1; // update index
		}
		else {
			copy_argv(node, NULL, argv, list_prev_pos, argc); // the last list
		}

		// if it is the first list, link it with the root
		if (list_prev_node == NULL) {
			init_node(node, LIST, root, NULL, NULL, NULL);
			root -> child = node;
		}
		else {
			init_node(node, LIST, NULL, NULL, list_prev_node, NULL);
			list_prev_node -> right = node; // update the adj node
		}

		// STEP-2: split line by '|'
		find_pos(node, NULL, pipe_pos, &pipe_ct, '|');
		int j = 0;
		pipe_prev_node = NULL;
		pipe_prev_pos = 0;
		do {
			node_in = make_node();

			if (pipe_ct == 0) {
				copy_argv(node_in, NULL, node -> argv, 0, node -> argc);
			}
			else if (j < pipe_ct) {
				copy_argv(node_in, NULL, node -> argv, pipe_prev_pos, pipe_pos[j]);
				pipe_prev_pos = pipe_pos[j] + 1;
			}
			else {
				copy_argv(node_in, NULL, node -> argv, pipe_prev_pos, node -> argc);
			}

			if (pipe_prev_node == NULL) {
				init_node(node_in, PIPE, node, NULL, NULL, NULL);
				node -> child = node_in;
			}
			else {
				init_node(node_in, PIPE, NULL, NULL, pipe_prev_node, NULL);
				pipe_prev_node -> right = node_in;
			}

			pipe_prev_node = node_in;
			j++;
		} while (j <= pipe_ct);

		list_prev_node = node;
		i++;
	} while (i <= list_ct);

	struct cmd* cmd = build_cmd_chain(root);

	return cmd;
}

int
Fork(void) {
	int pid;

	pid = fork();
	if(pid == -1)
		perror("error: fork()\n");
	
	return pid;
}

void
sigchld_handler(int sig) {
	int cpid = waitpid(-1, NULL, WNOHANG);
	if (cpid == child_pid_back) {
		printf("\n... child process (PID=%d) is waited/reaped\n", cpid);
	}
}

void
run_cmd(struct cmd *cmd) {
	int fd[2];

	int child_pid1;
	int child_pid2;
	struct execcmd *ecmd;
	struct redircmd *rcmd;
	struct pipecmd *pcmd;
	struct listcmd *lcmd;
	struct backcmd *bcmd;

	switch (cmd->type) {
	case EXEC:
		ecmd = (struct execcmd *)cmd;
		if (ecmd->argv[0] == 0) exit(0);

		execvp(ecmd->argv[0], (ecmd->argv));
		printf("exec %s failed\n", ecmd->argv[0]);

		// exit(0);
		break;
	case REDIR:
		rcmd = (struct redircmd*) cmd;

		setbuf(stdout, NULL);

		int fd_in = 0;
		int fd_out = 0;
		if (rcmd -> in_file != NULL) {
			fd_in = open(rcmd -> in_file, O_RDONLY);
			if (fd_in < 0) {
				printf("error: open()");
				exit(0);
			}
			dup2(fd_in, STDIN_FILENO);
		}
		if (rcmd -> out_file != NULL) {
			fd_out = open(rcmd -> out_file, O_CREAT|O_RDWR|O_TRUNC, S_IRUSR|S_IWUSR);
			if (fd_out < 0) {
				printf("error: open()");
				exit(0);
			}
			dup2(fd_out, STDOUT_FILENO);
		}

		run_cmd(rcmd -> cmd);

		// exit(0);
		break;
	case LIST:
		lcmd = (struct listcmd*) cmd;
		child_pid1 = Fork();

		if (child_pid1 == 0)
			run_cmd(lcmd->left);

		waitpid(child_pid1, NULL, 0);
		printf("\n... child process (PID=%d) is waited/reaped\n", child_pid1);
		run_cmd(lcmd->right);

		exit(0);
		break;
	case PIPE:
		pcmd = (struct pipecmd *)cmd;
		if (pipe(fd) < 0) {
			perror("error: pipe()");
		}
		child_pid1 = Fork();
		if (child_pid1 == 0) { // create a child process
			dup2(fd[1], STDOUT_FILENO); // connect stdout to the write end
			close(fd[0]); // close unused ends
			close(fd[1]);
			run_cmd(pcmd -> left);
		}

		// only parent executes this, no need for else
		child_pid2 = Fork();
		if (child_pid2 == 0) {
			dup2(fd[0], STDIN_FILENO); // connect stdin to the write end
			close(fd[0]); // close unused ends
			close(fd[1]);
			run_cmd(pcmd -> right);
		}

		close(fd[0]);
		close(fd[1]);
		waitpid(child_pid1, NULL, 0);
		waitpid(child_pid2, NULL, 0);
		printf("\n... child process (PID=%d) is waited/reaped\n", child_pid1);
		printf("\n... child process (PID=%d) is waited/reaped\n", child_pid2);

		exit(0);
		break;
	case BACK:
		bcmd = (struct backcmd *) cmd;
		
		// int ppid = getpid();
		child_pid_back = Fork();
		
		if(child_pid_back == 0){
			setpgid(0, 0); // put child into a new process group
			run_cmd(bcmd->cmd);
		}
		
		signal(SIGTTOU, SIG_IGN);
		setpgid(child_pid_back, child_pid_back);
		tcsetpgrp (0, parent_pid);
		
		// printf("shell> ");
		// exit(0);
	break;
	}
}

void
sigint_handler(int sig) {
	if (child_pid == 0) {
		printf("\nCtrl-C catched. But currently there is no foreground process running.\n");
	}
	else {
		kill(child_pid, SIGKILL);
		waitpid(child_pid, NULL, 0);
		printf("\n... child process (PID=%d) is waited/reaped\n", child_pid);
		child_pid = 0;
	}
	
	siglongjmp(ctrlc_buf, 1);
}

void 
loop(void) {
	int status = 1;

	struct cmd *cmd;

	int argc; // for parsing user input
	char argv[MAXARGS][MAXARGS]; // for getting user input
	char line[BUFSIZE];
	char *eline; // points to the end of line

	parent_pid = getpid(); // set to foreground in BACK cmd

	// signal(SIGCHLD, sigchld_handler);
	signal(SIGINT, sigint_handler);
	while (sigsetjmp(ctrlc_buf, 1) != 0); // jump back to here after SIGINT

	while (status) {
		// read input
		get_line(line);
		eline = line + strlen(line) - 2;
		
		split_line(line, eline, argv, &argc);
		
		// CASE 1: "exit"
		if (argc == 1 && strcmp(argv[0], "exit") == 0) 
			exit(0);
		
		// CASE 2: empty line
		if (argc == 0)
			continue;
		
		// CASE 3: normal
		cmd = build_parse_tree(argv, argc);
		
		child_pid = Fork();
		if (child_pid == 0)
			run_cmd(cmd);
		
		// reap current process
		if (child_pid == waitpid(child_pid, NULL, 0)) {
			// if it is a pipe cmd, not print out by the requirement of this assignment
			if (cmd -> type != PIPE) {
				printf("\n... child process (PID=%d) is waited/reaped\n", child_pid);
			}
		}

		// reap background process
		if (child_pid_back == waitpid(child_pid_back, NULL, WNOHANG)) {
			printf("\n... child process (PID=%d) is waited/reaped\n", child_pid_back);
		}

		// clear buffer
		memset(argv, 0, sizeof(argv[0][0]) * MAXARGS * MAXARGS);
		argc = 0;

		child_pid = 0;

		// free allocated resources
		free_cmds();
		free_nodes();
	}
}

int main(void){
	loop();
    return 0;
}