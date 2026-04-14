#include "file_browser.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QFileInfo>
#include <QDir>
#include <QSettings>
#include <QRegularExpression>
#include <QApplication>
#include <QTimer>

namespace qt_common {

// ── FileFilterProxy ──────────────────────────────────────────────────────────

FileFilterProxy::FileFilterProxy(QObject* parent)
    : QSortFilterProxyModel(parent) {
    setRecursiveFilteringEnabled(true);
}

void FileFilterProxy::setSearchText(const QString& text) {
    searchText_ = text.toLower();
    invalidateFilter();
}

bool FileFilterProxy::filterAcceptsRow(int row, const QModelIndex& parent) const {
    auto* model = qobject_cast<QFileSystemModel*>(sourceModel());
    if (!model) return true;

    QModelIndex idx = model->index(row, 0, parent);

    // Always show directories so the tree is navigable
    if (model->isDir(idx)) return true;

    // Apply base filter (file extension from QFileSystemModel::setNameFilters)
    if (!QSortFilterProxyModel::filterAcceptsRow(row, parent))
        return false;

    // Apply search text
    if (searchText_.isEmpty()) return true;
    QString name = model->fileName(idx).toLower();
    return name.contains(searchText_);
}

// ── FileBrowserDialog ────────────────────────────────────────────────────────

FileBrowserDialog::FileBrowserDialog(Mode mode, QWidget* parent)
    : QDialog(parent), mode_(mode) {
    buildUI();
    resize(800, 500);
}

void FileBrowserDialog::setWindowTitle(const QString& title) {
    QDialog::setWindowTitle(title);
}

void FileBrowserDialog::setNameFilters(const QStringList& filters) {
    filterLabels_ = filters;
    filterCombo_->clear();
    filterCombo_->addItems(filters);
    if (!filters.isEmpty())
        applyNameFilter(filters.first());
}

void FileBrowserDialog::setDefaultSuffix(const QString& suffix) {
    defaultSuffix_ = suffix;
}

void FileBrowserDialog::setDirectory(const QString& dir) {
    if (dir.isEmpty()) return;
    QDir d(dir);
    if (!d.exists()) return;

    fsModel_->setRootPath(dir);

    // Scroll to the directory after the model has loaded it
    QTimer::singleShot(100, this, [this, dir]() {
        QModelIndex srcIdx = fsModel_->index(dir);
        if (!srcIdx.isValid()) return;
        QModelIndex proxyIdx = proxy_->mapFromSource(srcIdx);
        if (!proxyIdx.isValid()) return;
        treeView_->scrollTo(proxyIdx, QAbstractItemView::PositionAtCenter);
        treeView_->expand(proxyIdx);
        treeView_->setCurrentIndex(proxyIdx);
    });
}

QString FileBrowserDialog::selectedFile() const {
    return selectedFile_;
}

QString FileBrowserDialog::directory() const {
    auto idx = treeView_->currentIndex();
    if (!idx.isValid()) return {};
    QModelIndex srcIdx = proxy_->mapToSource(idx);
    QString path = fsModel_->filePath(srcIdx);
    QFileInfo fi(path);
    return fi.isDir() ? path : fi.absolutePath();
}

void FileBrowserDialog::buildUI() {
    auto* mainLayout = new QVBoxLayout(this);

    // Search bar
    auto* searchLayout = new QHBoxLayout;
    searchLayout->addWidget(new QLabel("Search:"));
    searchBox_ = new QLineEdit;
    searchBox_->setPlaceholderText("Type to filter files...");
    searchBox_->setClearButtonEnabled(true);
    searchLayout->addWidget(searchBox_);
    mainLayout->addLayout(searchLayout);

    // File tree — rooted at filesystem root, fully navigable
    fsModel_ = new QFileSystemModel(this);
    fsModel_->setRootPath(QDir::rootPath());
    fsModel_->setReadOnly(mode_ == Open);

    proxy_ = new FileFilterProxy(this);
    proxy_->setSourceModel(fsModel_);

    treeView_ = new QTreeView;
    treeView_->setModel(proxy_);
    treeView_->setSortingEnabled(true);
    treeView_->sortByColumn(0, Qt::AscendingOrder);
    treeView_->setSelectionMode(QAbstractItemView::SingleSelection);
    treeView_->setAlternatingRowColors(true);

    // Show full filesystem from root
    QModelIndex rootProxy = proxy_->mapFromSource(fsModel_->index(QDir::rootPath()));
    treeView_->setRootIndex(rootProxy);

    // Column widths
    treeView_->header()->setStretchLastSection(false);
    treeView_->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    treeView_->setColumnWidth(1, 80);
    treeView_->setColumnWidth(2, 100);
    treeView_->setColumnWidth(3, 140);

    mainLayout->addWidget(treeView_, 1);

    // Filename entry
    auto* fileLayout = new QHBoxLayout;
    fileLayout->addWidget(new QLabel(mode_ == Save ? "Save as:" : "File:"));
    fileNameEdit_ = new QLineEdit;
    fileLayout->addWidget(fileNameEdit_);
    mainLayout->addLayout(fileLayout);

    // Filter + buttons
    auto* bottomLayout = new QHBoxLayout;
    filterCombo_ = new QComboBox;
    filterCombo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    bottomLayout->addWidget(filterCombo_);

    acceptBtn_ = new QPushButton(mode_ == Save ? "Save" : "Open");
    acceptBtn_->setDefault(true);
    cancelBtn_ = new QPushButton("Cancel");
    bottomLayout->addWidget(acceptBtn_);
    bottomLayout->addWidget(cancelBtn_);
    mainLayout->addLayout(bottomLayout);

    // Connections
    connect(searchBox_, &QLineEdit::textChanged, this, &FileBrowserDialog::onSearchChanged);
    connect(filterCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &FileBrowserDialog::onFilterChanged);
    connect(treeView_, &QTreeView::clicked, this, &FileBrowserDialog::onFileClicked);
    connect(treeView_, &QTreeView::doubleClicked, this, &FileBrowserDialog::onFileDoubleClicked);
    connect(acceptBtn_, &QPushButton::clicked, this, &FileBrowserDialog::onAccept);
    connect(cancelBtn_, &QPushButton::clicked, this, &QDialog::reject);
    connect(fileNameEdit_, &QLineEdit::returnPressed, this, &FileBrowserDialog::onAccept);
}

QStringList FileBrowserDialog::parseFilters(const QString& filterString) {
    QStringList result;
    for (const auto& part : filterString.split(";;")) {
        QString trimmed = part.trimmed();
        if (!trimmed.isEmpty())
            result << trimmed;
    }
    return result;
}

void FileBrowserDialog::applyNameFilter(const QString& filter) {
    static QRegularExpression re(R"(\(([^)]+)\))");
    auto match = re.match(filter);
    QStringList patterns;
    if (match.hasMatch()) {
        patterns = match.captured(1).split(' ', Qt::SkipEmptyParts);
    }
    nameFilters_ = patterns;
    fsModel_->setNameFilters(patterns);
    fsModel_->setNameFilterDisables(false);
}

void FileBrowserDialog::onSearchChanged(const QString& text) {
    proxy_->setSearchText(text);
}

void FileBrowserDialog::onFilterChanged(int index) {
    if (index >= 0 && index < filterLabels_.size())
        applyNameFilter(filterLabels_[index]);
}

void FileBrowserDialog::onFileClicked(const QModelIndex& index) {
    QModelIndex srcIdx = proxy_->mapToSource(index);
    if (!fsModel_->isDir(srcIdx)) {
        fileNameEdit_->setText(fsModel_->filePath(srcIdx));
    } else if (mode_ == Save) {
        // In save mode, clicking a directory sets the base path
        // while preserving any filename the user typed
        QString current = fileNameEdit_->text().trimmed();
        QString justName = QFileInfo(current).fileName();
        if (justName.isEmpty()) justName = "untitled";
        fileNameEdit_->setText(fsModel_->filePath(srcIdx) + "/" + justName);
    }
}

void FileBrowserDialog::onFileDoubleClicked(const QModelIndex& index) {
    QModelIndex srcIdx = proxy_->mapToSource(index);
    if (!fsModel_->isDir(srcIdx)) {
        fileNameEdit_->setText(fsModel_->filePath(srcIdx));
        onAccept();
    }
}

void FileBrowserDialog::onAccept() {
    QString path = fileNameEdit_->text().trimmed();
    if (path.isEmpty()) return;

    // For save mode: if path is relative (no directory separator),
    // prepend the currently browsed directory
    if (mode_ == Save && !path.contains('/') && !path.contains('\\')) {
        QString dir = directory();
        if (!dir.isEmpty()) {
            path = dir + "/" + path;
        }
    }

    if (mode_ == Save && !defaultSuffix_.isEmpty()) {
        if (!path.contains('.')) {
            path += "." + defaultSuffix_;
        }
    }

    if (mode_ == Open) {
        QFileInfo fi(path);
        if (!fi.exists() || !fi.isFile()) return;
    }

    selectedFile_ = path;
    accept();
}

// ── Static convenience methods ───────────────────────────────────────────────

QString FileBrowserDialog::getOpenFileName(QWidget* parent,
                                            const QString& caption,
                                            const QString& dir,
                                            const QString& filter) {
    FileBrowserDialog dlg(Open, parent);
    dlg.setWindowTitle(caption);
    dlg.setNameFilters(dlg.parseFilters(filter));
    dlg.setDirectory(dir);

    if (dlg.exec() == QDialog::Accepted)
        return dlg.selectedFile();
    return {};
}

QString FileBrowserDialog::getSaveFileName(QWidget* parent,
                                            const QString& caption,
                                            const QString& dir,
                                            const QString& filter) {
    FileBrowserDialog dlg(Save, parent);
    dlg.setWindowTitle(caption);
    dlg.setNameFilters(dlg.parseFilters(filter));

    // dir might be a full file path — split into directory and filename
    QFileInfo fi(dir);
    if (fi.isFile() || (!fi.exists() && !fi.suffix().isEmpty())) {
        dlg.setDirectory(fi.absolutePath());
        dlg.fileNameEdit_->setText(fi.absoluteFilePath());
    } else {
        dlg.setDirectory(dir);
    }

    if (dlg.exec() == QDialog::Accepted)
        return dlg.selectedFile();
    return {};
}

} // namespace qt_common
