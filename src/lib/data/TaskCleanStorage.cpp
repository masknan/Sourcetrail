#include "data/TaskCleanStorage.h"

#include "component/view/DialogView.h"
#include "data/storage/PersistentStorage.h"
#include "utility/file/FilePath.h"
#include "utility/scheduling/Blackboard.h"
#include "utility/utility.h"
#include "Application.h"

TaskCleanStorage::TaskCleanStorage(
	std::weak_ptr<PersistentStorage> storage, const std::vector<FilePath>& filePaths, bool clearAllErrors
)
	: m_storage(storage)
	, m_filePaths(filePaths)
	, m_clearAllErrors(clearAllErrors)
{
}

void TaskCleanStorage::doEnter(std::shared_ptr<Blackboard> blackboard)
{
	Application::getInstance()->getDialogView(DialogView::UseCase::INDEXING)->showUnknownProgressDialog(
		L"Clearing Files", std::to_wstring(m_filePaths.size()) + L" Files");

	m_start = utility::durationStart();

	if (!m_filePaths.empty() || m_clearAllErrors)
	{
		if (std::shared_ptr<PersistentStorage> storage = m_storage.lock())
		{
			storage->setMode(SqliteIndexStorage::STORAGE_MODE_CLEAR);
		}
	}
}

Task::TaskState TaskCleanStorage::doUpdate(std::shared_ptr<Blackboard> blackboard)
{
	if (std::shared_ptr<PersistentStorage> storage = m_storage.lock())
	{
		if (m_clearAllErrors)
		{
			storage->clearAllErrors();
		}

		storage->clearFileElements(m_filePaths, [=](int progress)
			{
				Application::getInstance()->getDialogView(DialogView::UseCase::INDEXING)->showProgressDialog(
					L"Clearing", std::to_wstring(m_filePaths.size()) + L" Files", progress);
			}
		);
	}

	m_filePaths.clear();

	return STATE_SUCCESS;
}

void TaskCleanStorage::doExit(std::shared_ptr<Blackboard> blackboard)
{
	blackboard->set("clear_time", utility::duration(m_start));

	Application::getInstance()->getDialogView(DialogView::UseCase::INDEXING)->hideProgressDialog();
}

void TaskCleanStorage::doReset(std::shared_ptr<Blackboard> blackboard)
{
}
