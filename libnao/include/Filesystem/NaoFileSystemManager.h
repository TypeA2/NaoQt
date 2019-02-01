/*
    This file is part of libnao.

    libnao is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libnao is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with libnao.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "libnao.h"

#define NaoFSM NaoFileSystemManager::global_instance()

class NaoFileSystemManager {

    class NotifyerFunctionBase {
        public:
        virtual void operator()() = 0;
        virtual ~NotifyerFunctionBase() = default;
    };

    template <class T>
    class NotifyerFunction : public NotifyerFunctionBase {
        public:
        NotifyerFunction(T* context, void (T::* member)())
            : _m_context(context)
            , _m_member(member) {

        }

        NotifyerFunction() = delete;

        void operator()() override {
            (_m_context->*_m_member)();
        }

        private:

        T* _m_context;
        void (T::* _m_member)();
    };

    public:
    // Global instance
    LIBNAO_API static NaoFileSystemManager& global_instance();

    LIBNAO_API bool init(const NaoString& root_dir);
    LIBNAO_API bool move(const NaoString& target);

    template <class T>
    void add_change_handler(T* context, void (T::* member)()) {
        add_change_handler(new NotifyerFunction(context, member));
    }

    LIBNAO_API NaoObject* current_object() const;
    LIBNAO_API const NaoString& last_error() const;

    private:

    LIBNAO_API void add_change_handler(NotifyerFunctionBase* func);

    NaoFileSystemManager();

    class NFSMPrivate;
    std::unique_ptr<NFSMPrivate> d_ptr;
};
