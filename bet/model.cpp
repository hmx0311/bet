#include "framework.h"
#include "model.h"

#include "common.h"

using namespace std;

constexpr double DBL_PRECISION_COMPENSATE = 1 + 40 * DBL_EPSILON;

Bet::Bet(double odds, int amount) :amount(amount), profit(amount* config.cut* odds* DBL_PRECISION_COMPENSATE)
{
	_stprintf(show, _T("%0.1f  %7d"), odds, amount);
}

Banker::Banker(double odds, int amount) : odds(odds), amount(amount), maxBought(int(amount* DBL_PRECISION_COMPENSATE / odds)* odds* DBL_PRECISION_COMPENSATE)
{
	_stprintf(show, _T("%0.1f %7d       0"), odds, amount);
}

void Banker::changeBought(int newBought)
{
	bought = newBought;
	profit = bought / odds * config.cut * DBL_PRECISION_COMPENSATE;
	_stprintf(show, _T("%0.1f %7d %7d"), odds, amount, bought);
}

long long Model::IntervalSumTree::getAmount(int index)
{
	return amount[index | (1 << INTERVAL_TREE_DEPTH)];
}

void Model::IntervalSumTree::deltaUpdate(int index, long long deltaAmount)
{
	for (index |= 1 << INTERVAL_TREE_DEPTH; index > 0; index >>= 1)
	{
		amount[index] += deltaAmount;
	}
}

long long Model::IntervalSumTree::getSum(int begin, int end)
{
	if (begin == end)
	{
		return 0;
	}
	if (begin > end)
	{
		return -getSum(end, begin);
	}
	begin |= 1 << INTERVAL_TREE_DEPTH;
	end |= 1 << INTERVAL_TREE_DEPTH;
	long long sum = amount[begin];
	for (; end - begin > 1; begin >>= 1, end >>= 1)
	{
		sum += amount[begin + 1] * !(begin & 1) + amount[end - 1] * (end & 1);
	}
	return sum;
}

long long Model::IntervalSumTree::total()
{
	return amount[1];
}

Model::Model() :haveClosing(config.defaultClosing) {}

void Model::addBet(bool side, Bet& bet)
{
	totalInvest += bet.amount;
	profit[side] += bet.profit;
	profit[!side] -= bet.amount;
	bets[side].push_back(bet);
}

void Model::addBanker(bool side, Banker& banker)
{
	totalInvest += banker.amount;
	int oddsIdx = round(10 * banker.odds);
	potentialProfit[side][0].deltaUpdate(oddsIdx, banker.maxBought / banker.odds * config.cut * DBL_PRECISION_COMPENSATE);
	potentialProfit[side][1].deltaUpdate(oddsIdx, -banker.maxBought);
	if (!haveClosing)
	{
		profit[!side] -= banker.maxBought;
	}
	bankers[side].push_back(banker);
}

const Banker& Model::allBought(bool side, int index)
{
	return changeBought(side, index, INT_MAX);
}

const Banker& Model::changeBought(bool side, int index, int amount)
{
	Banker& __restrict banker = bankers[side][index];
	if (amount > banker.maxBought)
	{
		amount = banker.maxBought;
	}
	if (amount == banker.bought)
	{
		return banker;
	}
	int oddsIdx = round(10 * banker.odds);
	if (haveClosing)
	{
		profit[!side] += banker.bought - amount;
	}
	potentialProfit[side][1].deltaUpdate(oddsIdx, amount - banker.bought);
	int porfitDiff = -banker.profit;
	banker.changeBought(amount);
	porfitDiff += banker.profit;
	profit[side] += porfitDiff;
	potentialProfit[side][0].deltaUpdate(oddsIdx, -porfitDiff);
	return banker;
}

void Model::moveBackBet(bool side, int index)
{
	swap(bets[side][index], bets[side][index + 1]);
}

void Model::moveBackBanker(bool side, int index)
{
	swap(bankers[side][index], bankers[side][index + 1]);
}

void Model::deleteBet(bool side, int index)
{
	vector<Bet>::iterator bet = bets[side].begin() + index;
	totalInvest -= bet->amount;
	profit[side] -= bet->profit;
	profit[!side] += bet->amount;
	bets[side].erase(bet);
}

void Model::deleteBanker(bool side, int index)
{
	vector<Banker>::iterator banker = bankers[side].begin() + index;
	totalInvest -= banker->amount;
	int oddsIdx = round(10 * banker->odds);
	profit[side] -= banker->profit;
	potentialProfit[side][0].deltaUpdate(oddsIdx, banker->profit - int(int(banker->amount / banker->odds) * config.cut));
	profit[!side] += haveClosing ? banker->bought : banker->maxBought;
	potentialProfit[side][1].deltaUpdate(oddsIdx, banker->maxBought - banker->bought);
	bankers[side].erase(banker);
}

void Model::reset()
{
	totalInvest = 0;
	for (int i = 0; i < 2; i++)
	{
		profit[i] = 0;
		bets[i].clear();
		bankers[i].clear();
	}
	memset(potentialProfit, 0, 2 * 2 * sizeof(IntervalSumTree));
}

bool Model::changeClosing()
{
	haveClosing = !haveClosing;
	if (potentialProfit[0][1].total() == 0 && potentialProfit[1][1].total() == 0)
	{
		return false;
	}
	for (int i = 0; i < 2; i++)
	{
		profit[!i] += haveClosing ? -potentialProfit[i][1].total() : potentialProfit[i][1].total();
	}
	return true;
}

pair<long long, long long> Model::calcAimAmountBalance(bool isBet, double odds)
{
	bool side = profit[0] > profit[1];
	if (profit[!side] - profit[side] <= MIN_AMOUNT)
	{
		return { 0,0 };
	}
	long long difference = -profit[side];
	long long balancedProfit = profit[!side];
	if (isBet)
	{
		if (haveClosing)
		{
			difference -= potentialProfit[!side][1].getSum(round(10 * odds), 100);
		}
		balancedProfit += potentialProfit[!side][0].getSum(round(10 * odds), 100);
	}
	else
	{
		difference -= potentialProfit[side][0].getSum(round(10 * odds), 100);
		if (haveClosing)
		{
			balancedProfit += potentialProfit[side][1].getSum(round(10 * odds), 100);
		}
	}
	difference += balancedProfit;
	long long aimAmount = round(difference / (config.cut * (isBet ? odds : 1 / odds) + 1));
	if (aimAmount < MIN_AMOUNT)
	{
		return { 0,0 };
	}
	balancedProfit -= aimAmount;
	if (!isBet)
	{
		for (long long temp = aimAmount; long long(long long(aimAmount / odds * DBL_PRECISION_COMPENSATE) * odds) < temp; aimAmount++);
	}
	return { aimAmount, balancedProfit };
}

void Model::calcReferenceOdds(long long initialAmount, double winProb, double winProbError, double* __restrict referenceOdds)
{
	winProbError *= (winProb > 0.5 ? 1 - winProb : winProb);
	for (int i = 0; i < 2; i++)
	{
		long long winAmount = initialAmount + profit[i];
		long long loseAmount = initialAmount + profit[!i];
		int lastOdds10 = 100;
		double correctedWinProb = winProb - winProbError;
		for (int j = 0; j < 2;)
		{
			int odds10 = 10 / DBL_PRECISION_COMPENSATE * config.cut * (correctedWinProb * loseAmount) / ((1 - correctedWinProb) * winAmount);
			if (odds10 > 99)
			{
				odds10 = 99;
			}
			if (odds10 == lastOdds10)
			{
				referenceOdds[4 * i + j] = odds10 * 0.1;
				j++;
				correctedWinProb = winProb;
				continue;
			}
			winAmount += potentialProfit[i][0].getSum(odds10, lastOdds10);
			if (haveClosing)
			{
				loseAmount += potentialProfit[i][1].getSum(odds10, lastOdds10);
			}
			lastOdds10 = odds10;
		}
		if (loseAmount - winAmount <= MIN_AMOUNT)
		{
			referenceOdds[4 * i + 1] = 0;
		}
		winAmount = initialAmount + profit[i];
		loseAmount = initialAmount + profit[!i];
		lastOdds10 = 100;
		correctedWinProb = winProb - winProbError;
		for (int j = 2; j < 4;)
		{
			int odds10 = ceil(10 / config.cut * ((1 - correctedWinProb) * winAmount) / (correctedWinProb * loseAmount));
			if (odds10 > 99)
			{
				j++;
				correctedWinProb = winProb;
				continue;
			}
			if (odds10 == lastOdds10)
			{
				referenceOdds[4 * i + j] = odds10 * 0.1;
				j++;
				correctedWinProb = winProb;
				continue;
			}
			if (haveClosing)
			{
				winAmount += potentialProfit[!i][1].getSum(odds10, lastOdds10);
			}
			loseAmount += potentialProfit[!i][0].getSum(odds10, lastOdds10);
			lastOdds10 = odds10;
		}
		if (loseAmount - winAmount <= MIN_AMOUNT)
		{
			referenceOdds[4 * i + 3] = 0;
		}
		winProb = 1 - winProb;
	}
}

long long Model::calcAimAmountProb(long long initialAmount, double winProb, double winProbError, bool side, bool isBet, double odds)
{
	winProb = side ? 1 - winProb : winProb;
	winProbError *= (winProb > 0.5 ? 1 - winProb : winProb);
	double lowerWinProb = winProb - winProbError;
	double upperWinProb = winProb;
	long long winAmount = initialAmount + profit[side];
	long long loseAmount = initialAmount + profit[!side];
	if (isBet)
	{
		if (haveClosing)
		{
			winAmount += potentialProfit[!side][1].getSum(round(10 * odds), 100);
		}
		loseAmount += potentialProfit[!side][0].getSum(round(10 * odds), 100);
	}
	else
	{
		winAmount += potentialProfit[side][0].getSum(round(10 * odds), 100);
		if (haveClosing)
		{
			loseAmount += potentialProfit[side][1].getSum(round(10 * odds), 100);
		}
	}
	double equivalentOdds = config.cut * (isBet ? odds : 1 / odds);
	long long aimAmount;
	if (loseAmount - winAmount <= MIN_AMOUNT)
	{
		aimAmount = round(lowerWinProb * loseAmount - (1 - lowerWinProb) / equivalentOdds * winAmount);
	}
	else
	{
		long long lowerAmount = round(lowerWinProb * loseAmount - (1 - lowerWinProb) / equivalentOdds * winAmount);
		long long upperAmount = round(upperWinProb * loseAmount - (1 - upperWinProb) / equivalentOdds * winAmount);
		long long balanceAmount = round((loseAmount - winAmount) / (equivalentOdds + 1));
		aimAmount = lowerAmount > balanceAmount ? lowerAmount : upperAmount > balanceAmount ? balanceAmount : upperAmount;
	}
	if (aimAmount < 0)
	{
		return 0;
	}
	if (!isBet)
	{
		for (long long temp = aimAmount; long long(long long(aimAmount / odds * DBL_PRECISION_COMPENSATE) * odds) < temp; aimAmount++);
	}
	return aimAmount;
}

long long Model::getProfit(bool side)
{
	return profit[side];
}

long long Model::getTotalInvest()
{
	return totalInvest;
}