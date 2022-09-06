#pragma once
#include <vector>

#define INTERVAL_TREE_DEPTH 7

class Bet
{
public:
	int amount = 0;
	int profit = 0;
	TCHAR show[13];

	Bet(double odds, int amount);
};

class Banker
{
public:
	int amount;
	double odds;
	int maxBought;
	int bought = 0;
	int profit = 0;
	TCHAR show[20];

	Banker(double odds, int amount);
	void changeBought(int newBought);
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

	long long totalInvest = 0;
	long long profit[2] = { 0,0 };
	std::vector<Bet> bets[2];
	std::vector<Banker> bankers[2];
	bool haveClosing;
	//[side][0]:被买后获胜收益
	//[side][1]:可被买数量（负值）
	IntervalSumTree potentialProfit[2][2];

public:
	Model();
	void addBet(bool side, Bet& bet);
	void addBanker(bool side, Banker& banker);
	const Banker& allBought(bool side, int index);
	const Banker& changeBought(bool side, int index, int amount);
	void moveBackBet(bool side, int index);
	void moveBackBanker(bool side, int index);
	void deleteBet(bool side, int index);
	void deleteBanker(bool side, int index);
	void reset();
	bool changeClosing();
	std::pair<long long, long long> calcAimAmountBalance(bool isBet, double odds);
	long long calcAimAmountProb(long long initialAmount, double winningProb, double winProbError, bool side, bool isBet, double odds);
	void calcReferenceOdds(long long initialAmount, double winningProb, double winProbError, double* __restrict referenceOdds);
	long long getProfit(bool side);
	long long getTotalInvest();
};