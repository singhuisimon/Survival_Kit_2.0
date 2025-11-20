/**
 * @file BTDecoratorNodes.cpp
 * @brief Definition of Behaviour Tree Decorator Nodes classes
 * @author Amanda Leow Boon Suan (90%), Rio Shannon Yvon Leonardo (10%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */


#pragma once

#include "BTDecoratorNodes.h"
#include "ECS/Scene.h"

namespace Engine {

    //BTDecorator
    bool BTDecorator::CanHaveChildren() const { return true; }
    
    void BTDecorator::AddChild(std::shared_ptr<BTNode> child) {
        m_Child = child;
        m_ChildEntered = false;
    }
    
    void BTDecorator::RemoveChild(size_t index) {
        if (index == 0) {
            m_Child.reset();
            m_ChildEntered = false;
        }
    }
    
    std::vector<std::shared_ptr<BTNode>>& BTDecorator::GetChildren() {
        m_ChildVector.clear();
        if (m_Child) {
            m_ChildVector.push_back(m_Child);
        }
        return m_ChildVector;
    }
    
    const std::vector<std::shared_ptr<BTNode>>& BTDecorator::GetChildren() const {
        m_ChildVector.clear();
        if (m_Child) {
            m_ChildVector.push_back(m_Child);
        }
        return m_ChildVector;
    }
    
    void BTDecorator::Reset() {
        m_ChildEntered = false;
        if (m_Child) {
            m_Child->Reset();
        }
    }

    // FIXED: Implement FindChildIndex for decorators
    int BTDecorator::FindChildIndex(const std::shared_ptr<BTNode>& child) const {
        if (m_Child && m_Child == child) {
            return 0;
        }
        return -1;
    }

    // FIXED: Implement FindChildIndexByGUID for decorators
    int BTDecorator::FindChildIndexByGUID(xresource::instance_guid guid) const {
        if (m_Child && m_Child->GetGUID() == guid) {
            return 0;
        }
        return -1;
    }
    

    //BTInverter
    const char* BTInverter::GetTypeName() const { return "Inverter"; }
    
    BTStatus BTInverter::Execute(BTContext& context) {
        if (!m_Child) {
            return BTStatus::Failure;
        }

        // Check if entity still exists before resetting
        if (context.Entity && context.Scene) {
            auto& registry = context.Scene->GetRegistry();
            entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
            if (registry.valid(entityHandle)) {
                return BTStatus::Failure;
            }
        }

        //testing/trying it out.
        if (m_ChildEntered == false)
        {
            m_Child->OnEnter(context);
            m_ChildEntered = true;
        }
    
        BTStatus status = m_Child->Execute(context);
    

		//need figure out if i need to set the child entered to false again
		//as well as when to call onexit
        // If child completed, call OnExit
        if (status != BTStatus::Running) {
            m_Child->OnExit(context);
            m_ChildEntered = false;
        }


        if (status == BTStatus::Success) {
            return BTStatus::Failure;
        }
        else if (status == BTStatus::Failure) {
            return BTStatus::Success;
        }
    
        return BTStatus::Running;
    }
    

    //BTSucceeder
    const char* BTSucceeder::GetTypeName() const { return "Succeeder"; }
    
    BTStatus BTSucceeder::Execute(BTContext& context) {
        if (!m_Child) {
            return BTStatus::Success;
        }
    
        // Call OnEnter if this is the first time executing the child
        if (!m_ChildEntered) {
            m_Child->OnEnter(context);
            m_ChildEntered = true;
        }

        BTStatus status = m_Child->Execute(context);

        // If child completed, call OnExit
        if (status != BTStatus::Running) {
            m_Child->OnExit(context);
            m_ChildEntered = false;
            // Always return success regardless of child result
            return BTStatus::Success;
        }


        return BTStatus::Running;
        //return (status == BTStatus::Running) ? BTStatus::Running : BTStatus::Success;
    }
    

    //BTFailer
    const char* BTFailer::GetTypeName() const { return "Failer"; }
    
    BTStatus BTFailer::Execute(BTContext& context) {

        if (!m_Child) {
            return BTStatus::Failure;
        }

        // Call OnEnter if this is the first time executing the child
        if (!m_ChildEntered) {
            m_Child->OnEnter(context);
            m_ChildEntered = true;
        }
    
        BTStatus status = m_Child->Execute(context);

		//need to figure out if i need to set the child entered to false again as well as call onexit

        // If child completed, call OnExit
        if (status != BTStatus::Running) {
            m_Child->OnExit(context);
            m_ChildEntered = false;
            // Always return failure regardless of child result
            return BTStatus::Failure;
        }

        return BTStatus::Running;
        //return (status == BTStatus::Running) ? BTStatus::Running : BTStatus::Failure;
    }
    

    //BTRepeater
    BTRepeater::BTRepeater(int repeatCount)
        : m_RepeatCount(repeatCount), m_CurrentCount(0) {}
    
    const char* BTRepeater::GetTypeName() const { return "Repeater"; }
    
    BTStatus BTRepeater::Execute(BTContext& context) {
        if (!m_Child) {
            return BTStatus::Failure;
        }

        // Infinite repeat (-1 or negative values)
        if (m_RepeatCount < 0) {
            LOG_INFO("REPEATER: INF REPEATING");

            // Call OnEnter if this is the first time executing the child
            if (!m_ChildEntered) {
                m_Child->OnEnter(context);
                m_ChildEntered = true;
            }

            BTStatus status = m_Child->Execute(context);

            // If child completed (Success or Failure), reset it for next iteration
            if (status != BTStatus::Running) {

                m_Child->OnExit(context);

                // Check if entity still exists before resetting
                if (context.Entity && context.Scene) {
                    auto& registry = context.Scene->GetRegistry();
                    entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                    if (registry.valid(entityHandle)) {
                        m_Child->Reset(); // Reset for next iteration
                        m_ChildEntered = false;  // Reset for next iteration
                    }
                }
                //m_Child->Reset();
            }

            // Always return Running to keep the infinite loop going
            return BTStatus::Running;
        }

        //if there is issue haha....
        // Limited repeat - Execute ONE iteration per frame (not all at once!)
        if (m_CurrentCount < m_RepeatCount) {
            LOG_INFO("REPEATER: EXECUTING ONE ITERATION PER FRAME, CURRENT: ", m_CurrentCount, " ,REPEATCOUNT: ", m_RepeatCount);
            
            // Call OnEnter if this is the first time executing the child
            if (!m_ChildEntered) {
                m_Child->OnEnter(context);
                m_ChildEntered = true;
            }

            // Execute the child node
            BTStatus status = m_Child->Execute(context);

            // If child is still running, keep this iteration going
            if (status == BTStatus::Running) {
                return BTStatus::Running;
            }

            // Child completed this iteration
            m_Child->OnExit(context);

            // Child completed this iteration (Success or Failure)
            // Reset child for next iteration
            // Check if entity still exists before resetting
            if (context.Entity && context.Scene) {
                auto& registry = context.Scene->GetRegistry();
                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                if (registry.valid(entityHandle)) {
                    m_Child->Reset(); // Reset for next iteration
                }
            }
            //m_Child->Reset();

            // Increment the counter
            m_CurrentCount++;
            m_ChildEntered = false;

            // Check if we've completed all requested repeats
            if (m_CurrentCount >= m_RepeatCount) {
                // All done - return Success
                // NOTE: Do NOT reset m_CurrentCount to 0 here!
                // It will be reset when Reset() is called externally
                return BTStatus::Success;
            }

            // More iterations needed
            // Return Running so we continue on the next frame
            return BTStatus::Running;
        }

        // Already completed all repeats (shouldn't normally reach here)
        return BTStatus::Success;
    }

    void BTRepeater::Reset() {
        m_CurrentCount = 0;

        BTDecorator::Reset();
    }
    
    void BTRepeater::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "RepeatCount", std::to_string(m_RepeatCount) });
    }
    
    void BTRepeater::SetProperty(const std::string& name, const std::string& value) {
        if (name == "RepeatCount") {
            m_RepeatCount = std::stoi(value);
        }
    }

    //BTRepeatUntilFail
    const char* BTRepeatUntilFail::GetTypeName() const { return "RepeatUntilFail"; }
    
    BTStatus BTRepeatUntilFail::Execute(BTContext& context) {
        if (!m_Child) {
            return BTStatus::Failure;
        }

        // Call OnEnter if this is the first time executing the child
        if (!m_ChildEntered) {
            m_Child->OnEnter(context);
            m_ChildEntered = true;
        }
    
        BTStatus status = m_Child->Execute(context);
    
        if (status == BTStatus::Failure) {
            // Child failed - exit and return success
            m_Child->OnExit(context);
            m_ChildEntered = false;

            // Check if entity still exists before resetting
            if (context.Entity && context.Scene) {
                auto& registry = context.Scene->GetRegistry();
                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                if (registry.valid(entityHandle)) {
                    m_Child->Reset(); // Reset for next iteration
                }
            }
            //m_Child->Reset();
            return BTStatus::Success; // We succeed when child fails
        }
        else if (status == BTStatus::Success) {
            // Child succeeded - call OnExit, reset, and call OnEnter for next iteration
            m_Child->OnExit(context);

            // Check if entity still exists before resetting
            if (context.Entity && context.Scene) {
                auto& registry = context.Scene->GetRegistry();
                entt::entity entityHandle = static_cast<entt::entity>(*context.Entity);
                if (registry.valid(entityHandle)) {
                    m_Child->Reset(); // Reset for next iteration
                    m_ChildEntered = false;  // Will call OnEnter again next iteration
                }
            }
            //m_Child->Reset(); // Reset for next iteration
        }
    
        return BTStatus::Running;
    }
    

    //BTCooldown
    BTCooldown::BTCooldown(float cooldownTime)
        : m_CooldownTime(cooldownTime), m_TimeSinceLastRun(cooldownTime) {}
    
    const char* BTCooldown::GetTypeName() const { return "Cooldown"; }
    
    BTStatus BTCooldown::Execute(BTContext& context) {
        if (!m_Child) {
            return BTStatus::Failure;
        }
    
        m_TimeSinceLastRun += context.DeltaTime;
    
        // Still in cooldown
        if (m_TimeSinceLastRun < m_CooldownTime) {
            return BTStatus::Failure;
        }
    
        // Execute child
        BTStatus status = m_Child->Execute(context);
    
        // Reset cooldown when child completes
        if (status != BTStatus::Running) {
            m_TimeSinceLastRun = 0.0f;
        }
    
        return status;
    }
    
    void BTCooldown::Reset() {
        m_TimeSinceLastRun = m_CooldownTime; // Ready to execute
        BTDecorator::Reset();
    }
    
    void BTCooldown::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "CooldownTime", std::to_string(m_CooldownTime) });
    }
    
    void BTCooldown::SetProperty(const std::string& name, const std::string& value) {
        if (name == "CooldownTime") {
            m_CooldownTime = std::stof(value);
        }
    }

} // namespace Engine
