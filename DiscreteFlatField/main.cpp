#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <time.h>
#include <iostream>
#include <cmath>
#include <random>
#include <vector>
#include <fstream>
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_real_distribution<double> dis(0, 1);
int random(int low, int high)
{
	std::uniform_int_distribution<> dist(low, high);
	return dist(gen);
}
class EpsilonGreedyBandit;
class UCBbandit;
class SoftMaxBandit;
template<class TypeBandit>
class Surround
{
public:
	//указатель для отображения среды
	sf::RenderWindow* field;
	int rows;
	int cols;
	TypeBandit** bandit_matrix;
	//координаты клетки, в которой находится агент
	int curr_x;
	int curr_y;
	//координаты начальной клетки
	int x0;
	int y0;
	//матрица наград и максимальная награда
	double** reward_matrix;
	double max_reward;
	//вероятность того, что клетка будет чёрной
	double noise;
	//количество пройденных количество шагов, сделанных до попадания напротивоположный край
		int n_steps;
	//скорость задержки отрисовки кадров в миллисекундах
	sf::Int32 speed;
	//сколько раз агент дошёл до противоположного края
	int n_finishes;
	//отображать ли поле
	bool animation;
	//сгенерировать награду за действие
	double throw_reward(int act)
	{
		if (curr_y == 0 && curr_x != cols - 1)
			act += 1;
		else if (curr_x == cols - 1 && curr_y != rows - 1)
			act += 2;
		else if (curr_y == rows - 1 && curr_x != 0)
			act += 3;
		act %= 4;
		if (act == 0)
		{
			curr_y -= 1;
			//circle.move(0, -70);
		}
		else if (act == 1)
		{
			//circle.move(70,0);
			curr_x += 1;
		}
		else if (act == 2)
		{
			//circle.move(0, 70);
			curr_y += 1;
		}
		else
		{
			//circle.move(-70, 0);
			curr_x -= 1;
		}
		//field->draw(circle);
		return reward_matrix[curr_y][curr_x];
	}
	void _draw()
	{
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				sf::RectangleShape cell(sf::Vector2f(70, 70));
				cell.setPosition(j * 70, i * 70);
				sf::Color color(reward_matrix[i][j] * 255 / max_reward, reward_matrix[i][j] * 255 /
					max_reward, reward_matrix[i][j] * 255 / max_reward);
				cell.setFillColor(color);
				cell.setOutlineColor(sf::Color::Black);
				cell.setOutlineThickness(0.5);
				sf::sleep(sf::milliseconds(1));
				field->draw(cell);
			}
		}
		sf::CircleShape circle(70 / 2);
		circle.setPosition(curr_x * 70, curr_y * 70);
		circle.setFillColor(sf::Color::Red);
		circle.setOutlineThickness(1);
		circle.setOutlineColor(sf::Color::Black);
		sf::sleep(sf::milliseconds(1));
		field->draw(circle);
		//sf::sleep(sf::milliseconds(1));
	}
	void _action()
	{
		bandit_matrix[curr_y][curr_x].take_action();
	}
public:
	Surround(sf::RenderWindow* window, bool Animation, int R = 8, int C = 8, int X = 0, int Y =
		0, double Noise = 0.1, sf::Int32 S = 1000 / 60, bool monitoring = false, double delta = 0, double c
		= 0, double eps = 0, double t = 0, double sigma = 0)
	{
		speed = S;
		field = window;
		rows = R;
		cols = C;
		curr_x = X;
		curr_y = Y;
		x0 = curr_x;
		y0 = curr_y;
		noise = Noise;
		n_steps = 0;
		n_finishes = 0;
		animation = Animation;
		/* sf::CircleShape c(70 / 2);
		circle = c;
		circle.setPosition(x0 * 70, y0 * 70);
		circle.setFillColor(sf::Color::Red);
		circle.setOutlineThickness(1);
		circle.setOutlineColor(sf::Color::Black);*/
		//field->draw(circle);
		bandit_matrix = new TypeBandit * [rows];
		reward_matrix = new double* [rows];
		for (int i = 0; i < rows; i++)
		{
			bandit_matrix[i] = new TypeBandit[cols];
			reward_matrix[i] = new double[cols];
		}
		std::cout << "reward_matrix: \n";
		for (int i = 0; i < rows; i++)
		{
			for (int j = 0; j < cols; j++)
			{
				int n_arms = 4;
				if (i == 0 || i == rows - 1)
					n_arms -= 1;
				if (j == 0 || j == cols - 1)
					n_arms -= 1;
				bandit_matrix[i][j] = TypeBandit(this, n_arms, monitoring, delta, c, eps, t, sigma);
				if (curr_x == 0 || curr_x == cols - 1)
				{
					reward_matrix[i][j] = abs(curr_x - j) + exp(-abs(curr_y - i)) * (dis(gen));
					if (j == cols - 1 && curr_x == 0 || j == 0 && curr_x == cols - 1)
					{
						reward_matrix[i][j] = abs(curr_x - j) + curr_x;
					}
				}
				else
				{
					reward_matrix[i][j] = abs(curr_y - i) + exp(-abs(curr_x - j)) * (dis(gen));
					if (i == 0 && curr_y == rows - 1 || i == rows - 1 && curr_y == 0)
					{
						reward_matrix[i][j] = abs(curr_y - i) + curr_y;
					}
				}
				if (reward_matrix[i][j] > max_reward)
					max_reward = reward_matrix[i][j];
				reward_matrix[i][j] *= 100;
				//рандомные клетки внутри поля нужно сделать чёрными, награда 0;
				double rand_num = dis(gen);
				if (rand_num < noise)
					reward_matrix[i][j] = 0;
				std::cout.width(4);
				std::cout.setf(std::ios::left);
				std::cout << reward_matrix[i][j] << ' ';
				;
			}
			std::cout << '\n';
		}
		max_reward *= 100;
		std::cout << " max reward: " << max_reward << '\n';
	}
	~Surround()
	{
		for (int i = 0; i < rows; i++)
		{
			delete[] reward_matrix[i];
		}
		delete[] reward_matrix;
	}
	double get_reward(int act)
	{
		n_steps += 1;
		double reward = throw_reward(act);
		if (reward == max_reward)
		{
			curr_x = x0;
			curr_y = y0;
			std::cout << n_steps << "\n";
			n_steps = 0;
			n_finishes++;
		}
		if (reward == 0)
			return -max_reward;
		return reward;
	}
	double get_max_reward() { return max_reward; }
	void start_game()
	{
		/*field->setVerticalSyncEnabled(true);
		field->setFramerateLimit(60);*/
		if (!animation)
		{
			field->clear();
			_draw();
			field->display();
		}
		while (field->isOpen())
		{
			sf::sleep(sf::milliseconds(1));
			sf::Event event;
			while (field->pollEvent(event))
			{
				if (event.type == sf::Event::Closed)
					field->close();
			}
			if (animation)
			{
				field->clear();
				_draw();
				sf::sleep(sf::milliseconds(speed));
				field->display();
			}
			if (n_finishes == 1000)
				field->close();
			_action();
			/*char c;
			std::cin >> c;*/
		}
	}
};
//абстрактный класс для бандитов
template <class TypeBandit>
class Bandit
{
protected:
	//среда, в которой находится агент
	Surround<TypeBandit>* s;
	//массив из оценок средней премии для каждого действия
	std::vector<double> Q;
	std::vector<int> action_counts;
	int n_actions;
	//количество ручек
	int n_arms;
	//вывод ли робот статистики в консоль
	bool monitoring;
	//эпсилон
	double eps;
	//параметры для понижения эписилон
	double c;
	double delta;
	//коэффициент для добавочного слагаемого
	double sigma;
	//гиперпараметр, "температура" в softmax стратегии
	double t;
	//после каждого выбора ручки агент может поменять стратегию
	virtual void change_strategy(int act, double reward) = 0;
public:
	Bandit()
	{
		s = NULL;
		std::vector<double> vec;
		Q = vec;
		std::vector<int> vec2;
		action_counts = vec2;
		n_arms = n_actions = eps = c = delta = sigma = t = 0;
		monitoring = false;
	}
	//выбрать ручку согласно стратегии
	virtual int take_action() = 0;
};
//эпислон-жадная стратегия
class EpsilonGreedyBandit : public Bandit<EpsilonGreedyBandit>
{
private:
	/*
//эпсилон
double eps;
//параметры для понижения эписилон
double c;
double delta;*/
	void change_strategy(int act, double reward)
	{
		/*bool explored = true;
		for (int i = 0; i < n_arms; i++)
		{
		if (Q[i] == 0)
		{
		explored = false;
		break;
		}
		}*/
		n_actions += 1;
		//if (explored)
		if (c != 0 && delta != 0)
			eps = c * n_arms / (pow(delta, 2) * n_actions) < 1 ? c * n_arms / (pow(delta, 2) *
				n_actions) : 1;
		if (monitoring)
		{
			std::cout << "\naction_counts before: " << action_counts[act] << '\n';
		}
		double current = Q[act];
		Q[act] = (current * action_counts[act] + reward) / (action_counts[act] + 1);
		//Q[act] = (Q[act] * action_counts[act] + reward) / (action_counts[act] + 1);
		action_counts[act] += 1;
	}
public:
	EpsilonGreedyBandit(Surround<EpsilonGreedyBandit>* S = NULL, int N = 4, bool flag =
		false, double C = 0.3, double d = 0.5, double epsilon = 1, double T = 100, double sigma = 2)
		:Bandit<EpsilonGreedyBandit>()
	{
		monitoring = flag;
		eps = epsilon;
		s = S;
		n_arms = N;
		c = C;
		delta = d;
		n_actions = 0;
		std::vector<double> vec(n_arms);
		Q = vec;
		//Q = new double[N];
		std::vector<int> vec2(n_arms);
		action_counts = vec2;
		//std::cout << "class has been created)\n";
	}
	int take_action()
	{
		double rand_num = dis(gen);
		if (monitoring)
		{
			std::cout << "\nrand_num: " << rand_num << " Q:" << " ";
			for (int i = 0; i < n_arms; i++)
				std::cout << Q[i] << " ";
			std::cout << " eps:" << eps;
		}
		int action = rand_num < eps ? random(0, n_arms - 1) : std::distance(Q.begin(),
			std::max_element(Q.begin(), Q.end()));
		double reward = s->get_reward(action);
		change_strategy(action, reward);
		return action;
	}
};
class UCBbandit : public Bandit<UCBbandit>
{
private:
	/*
	//коэффициент для добавочного слагаемого
	double sigma;
	*/
	void change_strategy(int act, double reward)
	{
		Q[act] = (Q[act] * action_counts[act] + reward) / (action_counts[act] + 1);
		action_counts[act]++;
		n_actions++;
		/*std::cout << "\n";
		for (int i = 0; i < n_arms; i++)
		std::cout << Q[i] << ' ';
		std::cout << "\tN_ACTIONS:\t" << n_actions;*/
	}
public:
	UCBbandit(Surround<UCBbandit>* S = NULL, int N = 4, bool flag = false, double C = 0.3,
		double d = 0.5, double epsilon = 1, double T = 42, double Sigma = 2) :Bandit<UCBbandit>()
	{
		s = S;
		n_arms = N;
		monitoring = flag;
		sigma = Sigma;
		n_actions = 1;
		std::vector<double> vec(n_arms);
		Q = vec;
		//std::vector<int> (n_arms);
		action_counts = std::vector<int>(n_arms);
	}
	int take_action()
	{
		int max_arm = 0;
		double max_arm_ucb = 0;
		for (int i = 0; i < n_arms; i++)
		{
			if (Q[i] == 0)
			{
				max_arm = random(0, n_arms - 1);
				break;
			}
			else
			{
				double ucb = Q[i] + sigma * sqrt(2 * log(n_actions) / action_counts[i]);
				if (ucb > max_arm_ucb)
				{
					max_arm = i;
					max_arm_ucb = ucb;
				}
			}
		}
		double reward = s->get_reward(max_arm);
		change_strategy(max_arm, reward);
		return max_arm;
	}
};
class SoftMaxBandit : public Bandit<SoftMaxBandit>
{
protected:
	/*
	//гиперпараметр, "температура" в softmax стратегии
	double t;
	*/
	void change_strategy(int act, double reward)
	{
		Q[act] = (Q[act] * action_counts[act] + reward) / (action_counts[act] + 1);
		action_counts[act]++;
		n_actions++;
		/* std::cout << "\n";
		for (int i = 0; i < n_arms; i++)
		std::cout << Q[i] << ' ';
		std::cout << "\tN_ACTIONS:\t" << n_actions;*/
	}
public:
	SoftMaxBandit(Surround<SoftMaxBandit>* S = NULL, int N = 4, bool flag = false, double C
		= 0.3, double d = 0.5, double epsilon = 1, double T = 42, double Sigma = 2) :
		Bandit<SoftMaxBandit>()
	{
		s = S;
		n_arms = N;
		t = T;
		std::vector<double> vec(n_arms);
		Q = vec;
		action_counts = std::vector<int>(n_arms);
		n_actions = 0;
	}
	int take_action()
	{
		double sum_exp = 0;
		for (int i = 0; i < n_arms; i++)
			sum_exp += exp(Q[i] / t);
		std::vector<double> probabilities(n_arms);
		for (int i = 0; i < n_arms; i++)
			probabilities[i] = exp(Q[i] / t) / sum_exp;
		double rand_num = dis(gen);
		double sum_probabilities = 0;
		int action = 0;
		for (int i = 0; i < n_arms; i++)
		{
			sum_probabilities += probabilities[i];
			if (sum_probabilities >= rand_num)
			{
				action = i;
				break;
			}
		}
		double reward = s->get_reward(action);
		change_strategy(action, reward);
		return action;
	}
};
int main()
{
	int rows = 7;
	int cols = 8;
	int start_x = 0;
	int start_y = 4;
	sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(cols * 70, rows * 70),
		"Field");
	Surround<UCBbandit> s(window, false, rows, cols, start_x, start_y, 0.2, 0, false, 0.0, 1,
		0.1, 150, 2);
	s.start_game();
}
