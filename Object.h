
class docker;
class docker_hub;
class docker_node;

__interface __object
{
public:
	void Initilize();
	void Destory();
};

class object : public __object
{
public:
	object()
	{
		Initilize();
	}
	virtual ~object()
	{
		Destory();
	}
};

class dock abstract
{
public:
	dock()
		: use_count_(0)
	{}

	virtual ~dock()
	{
		assert(use_count_ == 0);
	}

	virtual void decrease_count();
	virtual void increase_count();

protected:


private:
	std::atomic<WORD> use_count_;
};

__interface __docker_hub : public __docker
{
public:
	docker_node make_node();
};

__interface __docker_node : public __docker
{
public:
	bool collect();
};

class docker_hub final : public __docker_hub
{
public:
	~docker_hub();
	docker_node make_node() override;

private:
	friend docker_node;

	docker_hub();

	virtual inline void decrease_count() override { ref_count_.fetch_add(1); }
	virtual inline void increase_count() override { ref_count_.fetch_sub(1); }

	docker* docker_;
	std::atomic<WORD> ref_count_;
};

class docker_node final : public __docker_node
{
public:
	~docker_node()
	{
		docker_hub_->decrease_count();
	}

	bool collect() { return false; }

private:

	docker_node();

	docker_hub* docker_hub_;
};

class docker
{
public:

private:
	docker(object* data)
		: data_(data), use_count_(0)
	{
		assert(data != nullptr);
		data_->Initilize();
	};

	virtual ~docker()
	{
		data_->Destory();
	};

	object* data_;
	WORD use_count_;
};