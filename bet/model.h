#pragma once
#include <vector>

#define MIN_NOTABLE_DIFF 6

#define INTERVAL_TREE_DEPTH 7

struct Bet
{
	int amount = 0;
	int profit = 0;

	Bet(double odds, int amount, double cut);
};

struct Banker
{
	int amount;
	double odds;
	int maxBought;
	int bought = 0;
	int profit = 0;

	Banker(double odds, int amount);
	void changeBought(int newBought, double cut);
};

class Model
{
private:
	class IntervalSumTree
	{
	private:
		long long amount[(1 << (INTERVAL_TREE_DEPTH + 1)) - 1]{};

	public:
		long long getAmount(int index);
		void deltaUpdate(int index, long long deltaAmount);
		long long getSum(int begin, int end);	//sum between [begin,end)
		long long total();
	};

	double cut;
	long long totalInvest = 0;
	long long profit[2] = { 0,0 };
	std::vector<Bet> bets[2];
	std::vector<Banker> bankers[2];
	bool haveClosing;
	//[side][0]:被买后获胜收益
	//[side][1]:可被买数量（负值）
	IntervalSumTree potentialProfit[2][2];

public:
	Model(double cut);
	void addBet(bool side, double odds, int amount);
	const Banker& addBanker(bool side, double odds, int amount);
	const Banker& allBought(bool side, int index);
	const Banker& changeBought(bool side, int index, int amount);
	void moveBet(bool side, int index, int newPos);
	void moveBanker(bool side, int index, int newPos);
	void deleteBet(bool side, int index);
	void deleteBanker(bool side, int index);
	void reset();
	bool changeClosing();
	std::pair<long long, long long> calcAimAmountBalance(bool isBet, double odds);
	void calcReferenceOdds(long long initialAmount, double winningProb, double winProbError, double* __restrict referenceOdds);
	long long calcAimAmountProb(long long initialAmount, double winningProb, double winProbError, bool side, bool isBet, double odds);
	double getCut();
	long long getProfit(bool side);
	long long getTotalInvest();
};