#pragma once
#include <vector>

#define MIN_NOTABLE_DIFF 7

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
		__int64 amount[(1 << (INTERVAL_TREE_DEPTH + 1)) - 1]{};

	public:
		__int64 getAmount(uint32_t index) const;
		void deltaUpdate(uint32_t index, __int64 deltaAmount);
		__int64 getSum(uint32_t begin, uint32_t end) const;	//sum between [begin,end)
		__int64 total() const;
	};

	double cut;
	__int64 totalInvest = 0;
	__int64 profit[2] = { 0,0 };
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
	std::pair<__int64, __int64> calcAimAmountBalance(bool isBet, double odds) const;
	void calcReferenceOdds(__int64 initialAmount, double winProb, double errorFactor, double* __restrict referenceOdds) const;
	__int64 calcAimAmountProb(__int64 initialAmount, double winProb, double errorFactor, bool side, bool isBet, double odds) const;
	double getCut() const;
	__int64 getProfit(bool side) const;
	__int64 getTotalInvest() const;
};