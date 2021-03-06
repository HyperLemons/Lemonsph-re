/*
 * Copyright (c) 2018-2020 Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <stratosphere.hpp>

namespace ams::erpt::srv {

    class ManagerImpl final : public erpt::sf::IManager, public util::IntrusiveListBaseNode<ManagerImpl> {
        private:
            os::SystemEvent system_event;
        public:
            ManagerImpl();
            ~ManagerImpl();
        private:
            void NotifyOne();
        public:
            static Result NotifyAll();
        public:
            virtual Result GetReportList(const ams::sf::OutBuffer &out_list, ReportType type_filter) override final;
            virtual Result GetEvent(ams::sf::OutCopyHandle out) override final;
            virtual Result CleanupReports() override final;
            virtual Result DeleteReport(const ReportId &report_id) override final;
            virtual Result GetStorageUsageStatistics(ams::sf::Out<StorageUsageStatistics> out) override final;
            virtual Result GetAttachmentList(const ams::sf::OutBuffer &out_buf, const ReportId &report_id) override final;
    };

}
