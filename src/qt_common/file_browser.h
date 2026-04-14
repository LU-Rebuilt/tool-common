#pragma once
// file_browser.h — File browser dialog with search/filter.
// Drop-in replacement for QFileDialog::getOpenFileName / getSaveFileName.

#include <QDialog>
#include <QLineEdit>
#include <QTreeView>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>

namespace qt_common {

// Proxy model that filters files by name substring (case-insensitive).
class FileFilterProxy : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit FileFilterProxy(QObject* parent = nullptr);
    void setSearchText(const QString& text);

protected:
    bool filterAcceptsRow(int row, const QModelIndex& parent) const override;

private:
    QString searchText_;
};

// File browser dialog with:
//   - Directory tree on the left
//   - Search/filter box at the top
//   - File type filter dropdown
//   - Persistent last-directory via QSettings
class FileBrowserDialog : public QDialog {
    Q_OBJECT
public:
    enum Mode { Open, Save };

    explicit FileBrowserDialog(Mode mode, QWidget* parent = nullptr);

    void setWindowTitle(const QString& title);
    void setNameFilters(const QStringList& filters);
    void setDefaultSuffix(const QString& suffix);
    void setDirectory(const QString& dir);

    QString selectedFile() const;
    QString directory() const;

    // Static convenience methods matching QFileDialog signatures.
    static QString getOpenFileName(QWidget* parent,
                                    const QString& caption,
                                    const QString& dir,
                                    const QString& filter);

    static QString getSaveFileName(QWidget* parent,
                                    const QString& caption,
                                    const QString& dir,
                                    const QString& filter);

private slots:
    void onSearchChanged(const QString& text);
    void onFilterChanged(int index);
    void onFileClicked(const QModelIndex& index);
    void onFileDoubleClicked(const QModelIndex& index);
    void onAccept();

private:
    void buildUI();
    QStringList parseFilters(const QString& filterString);
    void applyNameFilter(const QString& filter);

    Mode mode_;
    QLineEdit* searchBox_ = nullptr;
    QLineEdit* fileNameEdit_ = nullptr;
    QTreeView* treeView_ = nullptr;
    QFileSystemModel* fsModel_ = nullptr;
    FileFilterProxy* proxy_ = nullptr;
    QComboBox* filterCombo_ = nullptr;
    QPushButton* acceptBtn_ = nullptr;
    QPushButton* cancelBtn_ = nullptr;

    QString selectedFile_;
    QString defaultSuffix_;
    QStringList nameFilters_;       // e.g. ["*.nif", "*.kf"]
    QStringList filterLabels_;      // e.g. ["NIF Files (*.nif *.kf)", "All Files (*)"]
};

} // namespace qt_common
