#pragma once

#include <any>
#include <functional>
#include <mutex>
#include <list>

namespace tsignal {

	template <typename ... _ParamT>
	class ISlot
	{
	public:
		virtual void invoke(_ParamT ... params) = 0;

		virtual bool isSame(std::any obj, std::any method) = 0;

		virtual bool isSame(std::any method) = 0;

		virtual bool isObjectSame(std::any obj) = 0;
	};

	template <typename _ClassT, typename _RetT, typename ... _ParamT>
	class ObjectSlot : public ISlot<_ParamT ...>
	{
	public:
		typedef _RetT(_ClassT::* MethodType)(_ParamT ...) ;

		ObjectSlot(_ClassT *obj, MethodType method) :
			obj_(obj),
			method_(method)
		{}

		virtual void invoke(_ParamT ... params) override
		{
			std::mem_fn(method_)(*obj_, params ...);
		}

		virtual bool isSame(std::any any_obj, std::any any_method) override
		{
			if (any_obj.type() != typeid(_ClassT *)) {
				return false;
			}

			_ClassT *obj = std::any_cast<_ClassT *>(any_obj);
			if (obj != obj_) {
				return false;
			}

			if (any_method.type() != typeid(MethodType)) {
				return false;
			}

			MethodType method = std::any_cast<MethodType>(any_method);

			if (method != method_) {
				return false;
			}

			return true;
		}

		virtual bool isSame(std::any method) override
		{
			return false;
		}

		virtual bool isObjectSame(std::any any_obj) override
		{
			if (any_obj.type() != typeid(_ClassT *)) {
				return false;
			}

			_ClassT *obj = std::any_cast<_ClassT *>(any_obj);
			if (obj != obj_) {
				return false;
			}

			return true;
		}

	private:
		_ClassT *obj_;
		MethodType method_;
	};

	template <typename _RetT, typename ... _ParamT>
	class MethodSlot : public ISlot<_ParamT ...>
	{
	public:
		typedef _RetT(*MethodType)(_ParamT ...);

		MethodSlot(MethodType method) :
			method_(method)
		{}

		virtual void invoke(_ParamT ... params) override
		{
			method_(params ...);
		}

		virtual bool isSame(std::any, std::any) override
		{
			return false;
		}

		virtual bool isSame(std::any any_method) override
		{
			if (any_method.type() != typeid(MethodType)) {
				return false;
			}

			return (std::any_cast<MethodType>(any_method) == method_);
		}

		virtual bool isObjectSame(std::any) override
		{
			return false;
		}

	private:
		MethodType method_;
	};

	template <typename _ClassT, typename _RetT, typename ... _ParamT>
	class ObjectFunctorSlot : public ISlot<_ParamT ...>
	{
	public:
		ObjectFunctorSlot(_ClassT *obj, std::function<_RetT(_ParamT ...)> functor) :
			obj_(obj),
			functor_(functor)
		{}

		virtual void invoke(_ParamT ... params) override
		{
			functor_(params ...);
		}

		virtual bool isSame(std::any, std::any) override
		{
			return false;
		}

		virtual bool isSame(std::any any_method) override
		{
			return false;
		}

		virtual bool isObjectSame(std::any any_obj) override
		{
			if (any_obj.type() != typeid(_ClassT *)) {
				return false;
			}

			_ClassT *obj = std::any_cast<_ClassT *>(any_obj);
			if (obj != obj_) {
				return false;
			}

			return true;
		}

	private:
		_ClassT *obj_;
		std::function<_RetT(_ParamT ...)> functor_;
	};

	template <typename _RetT, typename ... _ParamT>
	class FunctorSlot : public ISlot<_ParamT ...>
	{
	public:
		FunctorSlot(std::function<_RetT(_ParamT ...)> functor) :
			functor_(functor)
		{
		}

		virtual void invoke(_ParamT ... params) override
		{
			functor_(params ...);
		}

		virtual bool isSame(std::any, std::any) override
		{
			return false;
		}

		virtual bool isSame(std::any any_method) override
		{
			return false;
		}

		virtual bool isObjectSame(std::any) override
		{
			return false;
		}

	private:
		std::function<_RetT(_ParamT ...)> functor_;
	};

	class Connection
	{
	public:
		Connection(std::function<void()> disconnect_functor) :
			disconnect_functor_(disconnect_functor)
		{
		}

		void disconnect()
		{
			disconnect_functor_();
		}

	private:
		std::function<void()> disconnect_functor_;
	};

	template <typename ... _ParamT>
	class Signal
	{
	public:
		typedef ISlot<_ParamT ...> SlotType;

		template <typename _ClassT, typename _RetT>
		Connection connect(_ClassT *obj, _RetT(_ClassT::*fun)(_ParamT ...))
		{
			std::shared_ptr<SlotType> slotter =
				std::make_shared<ObjectSlot<_ClassT, _RetT, _ParamT ...>>(obj, fun);

			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.push_back(slotter);

			return Connection([=]() {
				std::lock_guard<std::mutex> lg(mutex_);
				slot_list_.remove(slotter);
			});
		}

		template <typename _ClassT>
		Connection connect(_ClassT *obj, std::function<void (_ParamT ...)> fun)
		{
			std::shared_ptr<SlotType> slotter =
				std::make_shared<ObjectFunctorSlot<_ClassT, void, _ParamT ...>>(obj, fun);

			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.push_back(slotter);

			return Connection([=]() {
				std::lock_guard<std::mutex> lg(mutex_);
				slot_list_.remove(slotter);
			});
		}

		template <typename _RetT>
		Connection connect(_RetT(*fun)(_ParamT ...))
		{
			std::shared_ptr<SlotType> slotter =
				std::make_shared<MethodSlot< _RetT, _ParamT ...>>(fun);

			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.push_back(slotter);

			return Connection([=]() {
				std::lock_guard<std::mutex> lg(mutex_);
				slot_list_.remove(slotter);
			});
		}

		Connection connect(std::function<void (_ParamT ...)> fun)
		{
			std::shared_ptr<SlotType> slotter =
				std::make_shared<FunctorSlot< void, _ParamT ...>>(fun);

			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.push_back(slotter);

			return Connection([=]() {
				std::lock_guard<std::mutex> lg(mutex_);
				slot_list_.remove(slotter);
			});
		}

		template <typename _ClassT, typename _RetT>
		void disconnect(_ClassT *obj, _RetT(_ClassT::*fun)(_ParamT ...) )
		{
			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.remove_if([&](std::shared_ptr<SlotType> slot) {
				return slot->isSame(obj, fun);
			});
		}

		template <typename _ClassT>
		void disconnect(_ClassT *obj)
		{
			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.remove_if([&](std::shared_ptr<SlotType> slot) {
				return slot->isObjectSame(obj);
			});
		}

		template <typename _RetT>
		void disconnect(_RetT(*fun)(_ParamT ...) )
		{
			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.remove_if([&](std::shared_ptr<SlotType> slot) {
				return slot->isSame(fun);
			});
		}

		void disconnectAll()
		{
			std::lock_guard<std::mutex> lg(mutex_);
			slot_list_.clear();
		}

		void operator ()(_ParamT ... params)
		{
			std::lock_guard<std::mutex> lg(mutex_);
			for (auto slot : slot_list_) {
				slot->invoke(params ...);
			}
		}

	private:
		std::mutex mutex_;
		std::list<std::shared_ptr<SlotType>> slot_list_;
	};

} // namespace tsignal
