#pragma once

#include "Dog/Core.h"
#include "Events/Events.h"

namespace Dog {

	class DOG_API Layer {
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer();

		virtual void OnAttach() {}
		virtual void OnDetach() {}
		virtual void OnUpdate() {}
		virtual void OnEvent(Event& event) {}

		inline const std::string& GetName() const { return m_DebugName; }
	protected:
		// name only meant for debug purposes
		std::string m_DebugName;
	};
}