#pragma once
#include <functional>
#include <vector>

template<class ...Args>
class EventBroadcast
{
	typedef std::function<void(Args...)> pfunc_t;

public:
	EventBroadcast() {}
	EventBroadcast(EventBroadcast& other) = delete;
	EventBroadcast& operator=(EventBroadcast& other) = delete;

	void RegisterListener(pfunc_t funcPtr)
	{
		if (FindListener(funcPtr) == listeners.end())
		{
			listeners.push_back(funcPtr);
		}
	}

	void UnregisterListener(pfunc_t funcPtr)
	{
		auto it = FindListener(funcPtr);
		if (it != listeners.end())
		{
			listeners.erase(it);
		}
	}

	void Invoke(Args... args)
	{
		for (auto l : listeners)
		{
			l(args...);
		}
	}

private:
	auto FindListener(pfunc_t funcPtr)
	{
		for (int i = 0; i < listeners.size(); i++)
		{
			if (listeners[i].target<void(Args...)>() == funcPtr.target<void(Args...)>())
				return listeners.begin() + i;
		}

		return listeners.end();
	}

	std::vector<pfunc_t> listeners;
};