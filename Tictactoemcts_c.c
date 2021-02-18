#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#pragma warning(disable:4996)

#define BOARDSIZE 3
#define true  1
#define false 0
#define Computer 1
#define User     2
#define max(a, b) (((a) > (b)) ? (a) : (b))
#define min(a, b) (((a) < (b)) ? (a) : (b))

int CheckedCount = 0; //이미 둔 자리의 수 9가 되면 비긴 것 
int winner = 0; //Computer = 1, User = 2
int maxDepth = 9; //몇 수 앞까지 두어볼지를 결정 
int bestPosition[2] = { -1, -1 }; //구조체로 할까 하다 간단하게 배열로 사용 Mimimax 함수에서 사용 
int turn; //mcts에서 player 
int rating[3] = {0, 0, 0}; // mcts와 alpha beta간 대결에서 승률을 알아보기 위한 변수 

//1차원 숫자(1-9)로 입력된 값을 좌표 값으로 변환하기 위한 구조체 
typedef struct tagPoint {
	int row;
	int col;
}Point;

typedef struct tagNode {
	int player;
	int move;
	int visits;
	int size_of_e_pos; // empty postions의 개수 
	int size_of_c_nodes; // child node의 개수 
	int* empty_positions;
	double wins;
	struct tagNode** child_nodes; // Node*를 담을 포인터 배열 
	struct tagNode* parent;
}Node;

typedef struct tagState {
	int count;
	int winner;
	int turn;
	int board[BOARDSIZE][BOARDSIZE];
}State;

// 1~9까지의 숫자를 3X3 2차원 배열에서 좌표로 변환 
Point GetPoint(int position)
{
	Point p;
	p.row = position / BOARDSIZE;
	p.col = position % BOARDSIZE;
	return p;
}

// 입력이 1차원이고, 보드는 2차원으로 되어 있어 입력된 위치에 체크되어 있는지 검사 
int isFill(int(*board)[BOARDSIZE], int position)
{
	Point p = GetPoint(position);
	if (board[p.row][p.col])
	{
		return true;
	}
	return false;
}

// 보드에 체크할 위치를 숫자로 입력받기위한 함수 
int GetNumber(int(*board)[BOARDSIZE])
{
	char number;
	while (true)
	{
		printf("Which position do you want? Input number(1 ~ 9) : ");
		scanf(" %c", &number);
		if (number < '1' || number > '9')
		{
			printf("\nPlease enter the correct number.\n");
		}
		else if (isFill(board, number - '1'))
		{
			printf("Fill position, Please enter the other number\n");
		}
		else
		{
			break;
		}
	}
	return number - '1';
}

// 보드를 0으로 초기화 
void InitBoard(int(*board)[BOARDSIZE])
{
	int i, j;
	for (i = 0; i < BOARDSIZE; i++)
	{
		for (j = 0; j < BOARDSIZE; j++)
		{
			board[i][j] = 0;
		}
	}
}

// 보드의 상태를 화면에 표시 
void PrintBoard(int(*board)[BOARDSIZE])
{
	int i, j;
	char symbol[3] = { ' ', 'X', 'O' };
	for (i = 0; i < BOARDSIZE; i++)
	{
		for (j = 0; j < BOARDSIZE; j++)
		{
			if (board[i][j])
			{
				printf("%c ", symbol[board[i][j]]);
			}
			else
			{
				printf("%d ", i * BOARDSIZE + j + 1);
			}
		}
		printf("\n");
	}
	printf("\n");
}

void do_move(int(*board)[BOARDSIZE], int position)
{
	turn = 3 - turn;
	Point p = GetPoint(position);
	board[p.row][p.col] = turn;
	CheckedCount++;
}

// User 또는 Computer로부터 입력된 값을 보드에 저장 
void SetNumber(int(*board)[BOARDSIZE], int position, int player)
{
	Point p = GetPoint(position);
	board[p.row][p.col] = player;
	CheckedCount++;
}

// 주어진 위치를 0으로 복원  
void SetZero(int(*board)[BOARDSIZE], int position)
{
	Point p = GetPoint(position);
	board[p.row][p.col] = 0;
	CheckedCount--;
}

// 승리체크 
int isWin(int(*board)[BOARDSIZE], int player)
{
	if ((board[0][0] == player && board[0][0] == board[0][1] && board[0][1] == board[0][2]) || // 가로0 
		(board[1][0] == player && board[1][0] == board[1][1] && board[1][1] == board[1][2]) || // 가로1 
		(board[2][0] == player && board[2][0] == board[2][1] && board[2][1] == board[2][2]) || // 가로2 
		(board[0][0] == player && board[0][0] == board[1][0] && board[1][0] == board[2][0]) || // 세로1 
		(board[0][1] == player && board[0][1] == board[1][1] && board[1][1] == board[2][1]) || // 세로2 
		(board[0][2] == player && board[0][2] == board[1][2] && board[1][2] == board[2][2]) || // 세로3 
		(board[0][0] == player && board[0][0] == board[1][1] && board[1][1] == board[2][2]) || // 대각선1 
		(board[2][0] == player && board[2][0] == board[1][1] && board[1][1] == board[0][2]))  // 대각선2 
	{
		winner = player;
		return true;
	}

	return false;
}

int isDraw()
{
	if (CheckedCount == BOARDSIZE * BOARDSIZE)
	{
		return true;
	}
	return false;
}

int isFinish(int(*board)[BOARDSIZE], int player)
{
	if (isWin(board, player) || isWin(board, 3 - player) || isDraw())
	{
		return true;
	}
	return false;
}

// 게임이 끝이 났을 때의 점수  
int evaluation()
{
	int score = 0;
	if (winner == Computer)
	{
		score = 1;
	}
	else if (winner == User)
	{
		score = -1;
	}
	winner = 0;
	return score;
}

// 누군가 승리를 했거나 비겼나 확인하고 결과를 출력 
int isGameOver(int(*board)[BOARDSIZE], int player)
{
	char* Player[3] = { " ", "Compuer", "User" };

	if (isWin(board, player))
	{
		//printf("%s won!\n", Player[player]);
		rating[player]++;
	}
	else if (CheckedCount == BOARDSIZE * BOARDSIZE)
	{
		//printf("Draw!\n");
		rating[0]++;
	}
	else
	{
		return false;
	}
	return true;
}

// 보드의 빈곳을 위치값으로 변환하여 반환 배열의 끝에는 개수를 넣어준다. 
void GetEmptyPosition(int(*board)[BOARDSIZE], int* emptyPosition)
{
	int i, j, index = 0;
	for (i = 0; i < BOARDSIZE; i++)
	{
		for (j = 0; j < BOARDSIZE; j++)
		{
			if (board[i][j] == 0)
			{
				emptyPosition[index++] = i * BOARDSIZE + j;
			}
		}
	}
	emptyPosition[9] = index; //빈곳의 개수가 필요하므로 배열의 마지막에 표시해줌 
}

// 컴퓨터가 둘 위치를 찾기위한 함수 
void SetBestPosition(int pos, int score)
{
	if (bestPosition[1] < score)
	{
		bestPosition[0] = pos;
		bestPosition[1] = score;
	}
}

// 베스트 위치를 찾기위한 서치함수 
int Minimax(int depth, int(*board)[BOARDSIZE], int player, int alpha, int beta)
{
	if (depth == 0 || isWin(board, 3 - player) || CheckedCount == 9)
	{
		return evaluation();
	}

	int emptyPosition[10];
	GetEmptyPosition(board, emptyPosition);
	if (player == Computer)
	{
		int i, score, maxScore = -100;
		for (i = 0; i < emptyPosition[9]; i++)
		{
			SetNumber(board, emptyPosition[i], player);
			score = Minimax(depth - 1, board, User, alpha, beta);
			SetZero(board, emptyPosition[i]);
			maxScore = max(score, maxScore);
			alpha = max(maxScore, alpha);
			if (depth == maxDepth)
			{
				SetBestPosition(emptyPosition[i], maxScore);
				//printf("pos = %d, score = %d\n", emptyPosition[i] + 1, maxScore);
			}
			if (beta <= alpha)
			{
				break;
			}
		}
		return maxScore;
	}
	else
	{
		int i, score, minScore = 100;
		for (i = 0; i < emptyPosition[9]; i++)
		{
			SetNumber(board, emptyPosition[i], player);
			score = Minimax(depth - 1, board, Computer, alpha, beta);
			SetZero(board, emptyPosition[i]);
			minScore = min(score, minScore);
			beta = min(minScore, beta);
			if (beta <= alpha)
			{
				break;
			}
		}
		return minScore;
	}
}

int get_size_of_e_pos(int board[][3])
{
	int temp[10];
	GetEmptyPosition(board, temp);
	return temp[9];
}

int* get_empty_positions(int board[][3])
{
	int i, temp[10];
	int* e_pos;
	GetEmptyPosition(board, temp);
	if (temp[9]) {
		e_pos = (int*)malloc(sizeof(int) * temp[9]);
		for (i = 0; i < temp[9]; ++i)
		{
			e_pos[i] = temp[i];
		}
		return e_pos;
	}
	else {
		return NULL;
	}
}

Node** get_child_nodes(int size)
{
	Node** child;
	child = (Node**)malloc(sizeof(Node*) * size);
	return child;
}

Node* make_node(int move, int board[][3], Node* parent)
{
	Node* node;
	node = (Node*)malloc(sizeof(Node));

	node->player = turn;
	node->move = move;
	node->wins = 0;
	node->visits = 0;
	node->size_of_c_nodes = 0;
	node->size_of_e_pos = get_size_of_e_pos(board);
	node->empty_positions = get_empty_positions(board);
	node->child_nodes = get_child_nodes(node->size_of_e_pos);
	node->parent = parent;

	return node;
}

Node* add_child_node(int move, int board[][3], Node* p_node)
{
	Node* node = make_node(move, board, p_node);
	p_node->child_nodes[p_node->size_of_c_nodes++] = node;
	return node;
}

int get_ucbs(double* ucbs, int n)
{
	int i;
	int index = 0;
	double max = ucbs[0];
	for (i = 1; i < n; ++i)
	{
		if (max < ucbs[i])
		{
			max = ucbs[i];
			index = i;
		}
	}
	return index;
}

Node* choice_child(Node* node)
{
	double ucb[9];
	int i;
	Node* c;
	for (i = 0; i < node->size_of_c_nodes; ++i)
	{
		c = node->child_nodes[i];
		ucb[i] = (c->wins / c->visits) + sqrt(2 * log(node->visits) / c->visits);
	}
	return node->child_nodes[get_ucbs(ucb, node->size_of_c_nodes)];
}

void copy_board(int dst_board[][3], int src_board[][3])
{
	int i, j;
	for (i = 0; i < 3; ++i)
	{
		for (j = 0; j < 3; ++j)
		{
			dst_board[i][j] = src_board[i][j];
		}
	}
}

void copy_state(State* s, int board[][3])
{
	s->count = CheckedCount;
	s->winner = winner;
	s->turn = turn;
	copy_board(s->board, board);
}

void restore(State* s, int board[][3])
{
	CheckedCount = s->count;
	winner = s->winner;
	turn = s->turn;
	copy_board(board, s->board);
}

double get_result(int board[][3], int player)
{
	if (winner == player) {
		winner = 0;
		return 1.0;
	}
	else if (winner == 0) {
		return 0.5;
	}
	else {
		winner = 0;
		return 0.0;
	}

}

int get_best_move(Node* node)
{
	Node** child = node->child_nodes;
	int i, index = 0;
	int max = child[0]->visits;
	for (i = 1; i < node->size_of_c_nodes; ++i)
	{
		if (max < child[i]->visits) {
			max = child[i]->visits;
			index = i;
		}
	}
	//for (i = 0; i < node->size_of_c_nodes; ++i)
	//{
	//	printf("pos = %d, wins = %5.1lf, visits = %4d\n", child[i]->move + 1, child[i]->wins, child[i]->visits);
	//}
	return child[index]->move;
}

void free_mem(Node* node)
{
	int i;
	if (node->empty_positions) {
		free(node->empty_positions);
		node->empty_positions = NULL;
	}
	for (i = 0; i < node->size_of_c_nodes; ++i)
	{
		free_mem(node->child_nodes[i]);
		free(node->child_nodes[i]);
		node->child_nodes[i] = NULL; 
	}
	// child가 다 해제되었으니 child를 담고있는 배열도 해제 
	if (i == node->size_of_c_nodes)
	{
		free(node->child_nodes);
		node->child_nodes = NULL;
	}
}

int Mcts(State* s, int iter_num)
{
	State state;
	Node* root_node = make_node(-1, s->board, NULL);
	Node* node;
	int i, index, move, pos[10];
	double result;
	for (i = 0; i < iter_num; ++i)
	{
		node = root_node;
		state = *s;
		
		// Selection
		while (node->size_of_e_pos == 0 && node->size_of_c_nodes)
		{
			node = choice_child(node);
			do_move(state.board, node->move);
		}

		// Expantion
		if (node->size_of_e_pos && !isFinish(state.board, turn))
		{
			move = node->empty_positions[--node->size_of_e_pos];
			do_move(state.board, move);
			node = add_child_node(move, state.board, node);
		}
		
		// simulation(roolout)
		while (!isFinish(state.board, turn))
		{
			GetEmptyPosition(state.board, pos);
			index = rand() % pos[9];
			move = pos[index];
			do_move(state.board, move);
		}
		result = get_result(state.board, node->player);

		// backpropagation
		while (node != NULL)
		{
			node->wins += result;
			node->visits += 1;
			node = node->parent;
			if (result == 1.0) {
				result = 0.0;
			}
			else if (result == 0.0) {
				result = 1.0;
			}
		}
		restore(s, s->board);
	}
	move = get_best_move(root_node);
	free_mem(root_node);
	free(root_node);
	return move;
}

// 게임이 진행되는 함수 
int PlayGame(int(*board)[BOARDSIZE], int player)
{
	State state;
	if (player == Computer)
	{
		Minimax(maxDepth, board, player, -100, 100);
		// 한 수를 두고나면 베스트 값을 초기화해줘야 다음에 옳바른 값을 찾을 수 있슴 
		bestPosition[1] = -1;
		turn = 3 - turn;
		return bestPosition[0];
	}
	else
	{
		copy_state(&state, board);
		//return GetNumber(board);
		return Mcts(&state, pow(9 - CheckedCount, 4));
	}
}

// 한 게임 더할것인지를 결정하는 함수 
int isContinue()
{
	char yesno;
	printf("One more? (y/n) : ");
	yesno = getch();
	if (yesno == 'y' || yesno == 'Y')
	{
		printf("%c\n", yesno);
		return true;
	}
	return false;
}

// 게임이 시작될 때 초기화가 필요한 변수들을 모아서 초기화 해줌 
int InitGame(int(*board)[BOARDSIZE])
{
	int player;
	InitBoard(board);
	//printf("Tic Tac Toe v1.0\n");
	//PrintBoard(board);
	CheckedCount = 0;
	bestPosition[1] = -1;
	player = rand() % 2 + 1; // 선을 선택하기 위하여 
	return player;
}

int main(void)
{
	int i = 0, cnt = 1000;
	int player = 0;
	int position;
	int board[BOARDSIZE][BOARDSIZE];
	clock_t t1, t2;

	srand((unsigned)time(NULL));
	
	t1 = clock();
	player = InitGame(board);
	turn = 3 - player;
	while (true)
	{
		position = PlayGame(board, player);
		SetNumber(board, position, player);
		//PrintBoard(board);
		if (isGameOver(board, player))
		{
			if(++i >= cnt)
				break;
			
			//if (!isContinue())
			//{
			//	break;
			//}
			//else
			//{
			player = InitGame(board);
			turn = player;
			//}
		}
		player = 3 - player;
	}
	t2 = clock();
	printf("Draw : %d, alpha beta = %d, mcts = %d\n", rating[0], rating[1], rating[2]);
	printf("elipsed time of %d times = %.3fsec", cnt, (t2 - t1) / 1000.0);

	return 0;
}
