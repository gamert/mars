#ifndef TIMEMEASURE_H
#define TIMEMEASURE_H


#include <thread>
#include <mutex>
#include <map>
#include <vector>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif


typedef std::chrono::steady_clock::time_point CTimePoint;
typedef std::chrono::duration<double> CTimeDuration;

#define GetClock std::chrono::steady_clock::now


//时间测量
struct TimeMeasure_t
{
public:
	int m_id;
	std::vector<double> m_pings;	//second
	TimeMeasure_t(int id) :m_id(id)
	{
		m_pings.reserve(1024);
	}
	void Clear()
	{
		m_pings.clear();
	}
	//返回
	double Dump(bool blog = false)
	{
		//now() 获取当前时钟
		double dmin = 9999999;
		double dmax = 0; //dmax.zero();
		double dd = 0; //dd.zero();
		for (int i = 0; i < m_pings.size(); ++i)
		{
			if (dmin > m_pings[i])
				dmin = m_pings[i];

			if (dmax < m_pings[i])
				dmax = m_pings[i];

			dd += m_pings[i];
		}
		int count = max(1, m_pings.size());
		double d2 = dd / count;
		//计算方差:用来度量随机变量和其数学期望（即均值）之间的偏离程度
		double Variance = 0;
		for (int i = 0; i < m_pings.size(); ++i)
		{
			double dd = m_pings[i] - d2;
			Variance += dd * dd;
		}
		Variance = Variance / count;
		//标准差
		double SD = sqrtf(Variance);
		if (blog)
			printf("TimeMeasure[%d]:count=%d, avg=%llf,Variance=%llf,SD=%llf, [%llf,%llf] \n", m_id, count, d2, Variance, SD, dmin, dmax);
		return d2;
	}
};


class TimeMeasureGroup_t
{
public:
	std::map<int, TimeMeasure_t*> m_Measures;
	void Add(int id, double dura)
	{
		TimeMeasure_t* p;
		std::map<int, TimeMeasure_t*>::iterator itr = m_Measures.find(id);
		if (itr == m_Measures.end())
		{
			p = new TimeMeasure_t(id);
			m_Measures[id] = p;
		}
		else
		{
			p = itr->second;
		}
		p->m_pings.push_back(dura);
	}
	void Dump()
	{
		for each (auto var in m_Measures)
		{
			var.second->Dump();
		}
	}


};


#endif // !TIMEMEASURE_H