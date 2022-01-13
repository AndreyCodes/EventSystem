#include <iostream>
#include<coroutine>
#include<variant>
#include<deque>
#include<list>
#include<SFML/Graphics.hpp>
#include<chrono>
struct task
{
	struct promise_type;
	using handle = std::coroutine_handle<promise_type>;
	struct promise_type
	{
		std::suspend_never initial_suspend()
		{
			return {};
		}
		std::suspend_never final_suspend() noexcept
		{
			return {};
		}
		task get_return_object()
		{
			return handle::from_promise(*this);
		}
		void return_void() {}
		void unhandled_exception() noexcept {}

	};
	task(handle){}

};

class EventSystem final
{
private:
	template<class T>
	class Event//INTERFACE
	{
	protected:
		Event() = default;
	public:
		
		bool await_ready()
		{
			static_cast<T&>(*this).await_ready();
		}
		void await_suspend(std::coroutine_handle<task::promise_type> h)
		{
			static_cast<T&>(*this).await_suspend(h);
		}
		void await_resume()
		{
			static_cast<T&>(*this).await_resume();
		}
	};
public:

	template<class T>
	class DefaultUserInputEvent :public Event<DefaultUserInputEvent<T>>
	{
	protected:
		DefaultUserInputEvent() = default;
	public:
		using this_type = DefaultUserInputEvent;

		static T& get()
		{

			static T& this_ = []()->decltype(auto) {
													auto& res = events.emplace_back(std::in_place_type<T>);
													return std::get<T>(res);
												   }();
			return this_;
		}
		bool await_ready()
		{
			return false;
		}
		void await_suspend(std::coroutine_handle<task::promise_type> h)
		{
			coros.push_back(h);
		}
		void await_resume(){}

		void update()
		{
			static_cast<T&>(*this).update();
		}
		void resume()
		{
			std::list< std::coroutine_handle<task::promise_type>> temp = std::move(coros);

			for (auto& el : temp)
			{
				el.resume();
			}
		}

		//	 list: DONT WANT INVALIDATE ITERATORS
		std::list< std::coroutine_handle<task::promise_type>> coros;
	};

	class MouseLeftButtonPress : public DefaultUserInputEvent<MouseLeftButtonPress>
	{
	public:
		MouseLeftButtonPress() = default;
		void update()//CRTP overrides
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				resume();
			}
		}
	};
	class MouseRightButtonPress : public DefaultUserInputEvent<MouseRightButtonPress>
	{
	public:

		void update()//CRTP overrides
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
			{
				resume();
			}
		}
	};
	class Keyboard_R_Pressed : public DefaultUserInputEvent<Keyboard_R_Pressed>
	{
	public:

		void update()//CRTP overrides
		{
			if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
			{
				resume();
			}
		}
	};


	static void update()
	{
		for (auto& var_ev : events)
		{
			std::visit([](auto& ev)
				{
					ev.update();
				},var_ev);
		}
	}

private:
	using var_event_t = std::variant<MouseLeftButtonPress, MouseRightButtonPress, Keyboard_R_Pressed>; //any quantity of Types
	//	 deque: DONT WANT INVALIDATE REFERENCES TO VARIANTS
	static inline std::deque<var_event_t> events;


};

class Gun
{
public:
	Gun()
	{
		shoot(); //запуск корутины
		reload();
	}
private:
	int ammo = 5;
	task shoot()
	{
		using namespace std::chrono_literals;
		auto time_point_next = std::chrono::steady_clock::now()+2000ms;
		while (1)
		{
			co_await EventSystem::MouseLeftButtonPress::get();

			if (std::chrono::steady_clock::now() > time_point_next && ammo > 0)
			{
				//some game logic;
				ammo -= 1;
				std::cout << "shoot! " << ammo << " left\n";
				if (ammo == 0)
				{
					std::cout << "Please press R to reload\n";
				}
				time_point_next += 2000ms;
			}
		}
	}
	task reload()
	{
		using namespace std::chrono_literals;
		auto time_point_next = std::chrono::steady_clock::now() + 2000ms;
		while (1)
		{
			co_await EventSystem::Keyboard_R_Pressed::get();

			if (std::chrono::steady_clock::now() > time_point_next && ammo < 5)
			{
				//some game logic;
				ammo = 5;
				std::cout<< "Reload\n";
				time_point_next += 2000ms;
			}
		}
	}
};
int main()
{
	sf::RenderWindow window(sf::VideoMode(800, 600), "Test");
	window.setFramerateLimit(60);
	Gun a;
	std::cout << "press LeftMouse to shoot!\n";

	while (window.isOpen())
	{
		sf::Event ev;
		while (window.pollEvent(ev))
		{
			if (ev.type == sf::Event::Closed)
			{
				window.close();
			}
		}


		EventSystem::update();

	}
}
