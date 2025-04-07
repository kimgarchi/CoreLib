#pragma once

class JobBase;
class Thread;

class TaskBase abstract
{
public:

private:
};

class ActiveTask : public TaskBase
{

};

class PassiveTask : public TaskBase
{

};


class Task
{
public:
	Task(std::size_t thread_count, std::shared_ptr<JobBase> job_ptr);
	~Task();

	bool Stop(DWORD timeout = INFINITE);
	inline bool is_runable() const { return is_runable_.load(); }
	void Attach(std::size_t count);
	bool Deattach(std::size_t count, DWORD timeout = INFINITE);

	size_t thread_count() { return thread_que_.size(); }

private:
	const std::shared_ptr<JobBase> job_ptr_;
	std::deque<std::shared_ptr<Thread>> thread_que_;
	std::atomic_bool is_runable_;
};