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

	//返回avg
	double Dump(const char *prefix, bool blog = false)
	{
		std::vector<double> jitters;
		double avg = CaclQos(prefix, m_id, m_pings, jitters, blog);

		if (jitters.size() && blog)
		{
			std::vector<double> jitters2;
			double avg2 = CaclQos("		Jitter", m_id, jitters, jitters2, blog);
		}

		return avg;
	}

	//计算网络质量QOS:
	static double CaclQos(const char *prefix, int m_id,std::vector<double> &_pings, std::vector<double> &jitters, bool blog = false)
	{
		//now() 获取当前时钟
		double dmin = 9999999;
		double dmax = 0; //dmax.zero();
		double dd = 0; //dd.zero();
		for (int i = 0; i < _pings.size(); ++i)
		{
			if (dmin > _pings[i])
				dmin = _pings[i];

			if (dmax < _pings[i])
				dmax = _pings[i];

			if (i > 0)
			{
				double dd = fabs(_pings[i] - _pings[i - 1]);
				jitters.push_back(dd);
			}

			dd += _pings[i];
		}
		int count = max(1, _pings.size());
		//均值
		double avg = dd / count;
		//计算方差:用来度量随机变量和其数学期望（即均值）之间的偏离程度
		double Variance = 0;
		for (int i = 0; i < _pings.size(); ++i)
		{
			double dd = _pings[i] - avg;
			Variance += dd * dd;
		}
		Variance = Variance / count;
		//标准差
		double SD = sqrtf(Variance);
		if (blog)
			printf("%s[%d]:count=%d, avg=%llf,Variance=%llf,SD=%llf, [%llf,%llf] \n", prefix, m_id, count, avg, Variance, SD, dmin, dmax);
		return avg;
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
		//for each (auto var in m_Measures)
		//{
		//	var.second->Dump("TimeMeasureGroup");
		//}
	}


};


#endif // !TIMEMEASURE_H