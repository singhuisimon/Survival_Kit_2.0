/**
 * @file BTDecoratorNodes.h
 * @brief Decorator behaviour tree nodes (modify child behaviour)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTDecoratorNodes.h"

namespace Engine {

    //BTDecorator
    bool BTDecorator::CanHaveChildren() const { return true; }
    
    void BTDecorator::AddChild(std::shared_ptr<BTNode> child) {
        m_Child = child;
    }
    
    void BTDecorator::RemoveChild(size_t index) {
        if (index == 0) {
            m_Child.reset();
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
        if (m_Child) {
            m_Child->Reset();
        }
    }
    

    //BTInverter
    const char* BTInverter::GetTypeName() const { return "Inverter"; }
    
    BTStatus BTInverter::Execute(BTContext& context) {
        if (!m_Child) {
            return BTStatus::Failure;
        }
    
        BTStatus status = m_Child->Execute(context);
    
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
    
        BTStatus status = m_Child->Execute(context);
        return (status == BTStatus::Running) ? BTStatus::Running : BTStatus::Success;
    }
    

    //BTFailer
    const char* BTFailer::GetTypeName() const { return "Failer"; }
    
    BTStatus BTFailer::Execute(BTContext& context) {
        if (!m_Child) {
            return BTStatus::Failure;
        }
    
        BTStatus status = m_Child->Execute(context);
        return (status == BTStatus::Running) ? BTStatus::Running : BTStatus::Failure;
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
            BTStatus status = m_Child->Execute(context);

            // If child completed (Success or Failure), reset it for next iteration
            if (status != BTStatus::Running) {
                m_Child->Reset();
            }

            // Always return Running to keep the infinite loop going
            return BTStatus::Running;
        }

        // Limited repeat - Execute ONE iteration per frame (not all at once!)
        if (m_CurrentCount < m_RepeatCount) {
            LOG_INFO("REPEATER: EXECUTING ONE ITERATION PER FRAME, CURRENT: ", m_CurrentCount, " ,REPEATCOUNT: ", m_RepeatCount);
            // Execute the child node
            BTStatus status = m_Child->Execute(context);

            // If child is still running, keep this iteration going
            if (status == BTStatus::Running) {
                return BTStatus::Running;
            }

            // Child completed this iteration (Success or Failure)
            // Reset child for next iteration
            m_Child->Reset();

            // Increment the counter
            m_CurrentCount++;

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
    
        BTStatus status = m_Child->Execute(context);
    
        if (status == BTStatus::Failure) {
            m_Child->Reset();
            return BTStatus::Success; // We succeed when child fails
        }
        else if (status == BTStatus::Success) {
            m_Child->Reset(); // Reset for next iteration
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
