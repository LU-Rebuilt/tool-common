#include "ldf_table_widget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <algorithm>

namespace lu_widgets {

static QString sanitize(const std::string& s) {
    QString qs = QString::fromLatin1(s.data(), static_cast<int>(s.size()));
    qs.replace(QChar('\0'), ", ");
    return qs;
}

struct LdfTypeInfo {
    uint8_t id;
    const char* name;
};

static constexpr LdfTypeInfo LDF_TYPES[] = {
    {0,  "WString"},
    {1,  "S32"},
    {3,  "Float"},
    {4,  "Double"},
    {5,  "U32"},
    {7,  "Bool"},
    {8,  "U64"},
    {9,  "ObjId"},
    {13, "Utf8"},
};

static constexpr int LDF_TYPE_COUNT = sizeof(LDF_TYPES) / sizeof(LDF_TYPES[0]);

static QString ldf_type_display(uint8_t type) {
    for (int i = 0; i < LDF_TYPE_COUNT; ++i) {
        if (LDF_TYPES[i].id == type)
            return QString("%1 (%2)").arg(LDF_TYPES[i].name).arg(type);
    }
    return QString("Unknown (%1)").arg(type);
}

static int ldf_type_to_combo_index(uint8_t type) {
    for (int i = 0; i < LDF_TYPE_COUNT; ++i) {
        if (LDF_TYPES[i].id == type) return i;
    }
    return 0;
}

class LdfTypeDelegate : public QStyledItemDelegate {
public:
    using QStyledItemDelegate::QStyledItemDelegate;

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem&,
                          const QModelIndex&) const override {
        auto* combo = new QComboBox(parent);
        for (int i = 0; i < LDF_TYPE_COUNT; ++i) {
            combo->addItem(QString("%1 (%2)").arg(LDF_TYPES[i].name).arg(LDF_TYPES[i].id),
                           LDF_TYPES[i].id);
        }
        return combo;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        auto* combo = static_cast<QComboBox*>(editor);
        QString text = index.data().toString();
        for (int i = 0; i < combo->count(); ++i) {
            if (combo->itemText(i) == text) {
                combo->setCurrentIndex(i);
                return;
            }
        }
        combo->setCurrentIndex(0);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model,
                      const QModelIndex& index) const override {
        auto* combo = static_cast<QComboBox*>(editor);
        model->setData(index, combo->currentText());
    }
};

LdfTableWidget::LdfTableWidget(QWidget* parent) : QWidget(parent) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    table_ = new QTableWidget;
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({"Key", "Type", "Value"});
    table_->horizontalHeader()->setStretchLastSection(true);
    table_->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);
    table_->setItemDelegateForColumn(1, new LdfTypeDelegate(table_));
    layout->addWidget(table_);

    auto* btn_layout = new QHBoxLayout;
    btn_layout->setContentsMargins(0, 0, 0, 0);
    add_btn_ = new QPushButton("Add Entry");
    remove_btn_ = new QPushButton("Remove Entry");
    btn_layout->addWidget(add_btn_);
    btn_layout->addWidget(remove_btn_);
    btn_layout->addStretch();
    layout->addLayout(btn_layout);

    connect(add_btn_, &QPushButton::clicked, this, &LdfTableWidget::on_add);
    connect(remove_btn_, &QPushButton::clicked, this, &LdfTableWidget::on_remove);
    connect(table_, &QTableWidget::cellChanged, this, [this](int row, int) {
        if (row >= 0 && row < table_->rowCount()) {
            QString key = table_->item(row, 0) ? table_->item(row, 0)->text() : "";
            int col = table_->columnCount() - 1;
            QString val = table_->item(row, col) ? table_->item(row, col)->text() : "";
            emit entry_edited(row, key, val);
        }
    });
}

void LdfTableWidget::set_entries(const std::vector<lu::assets::LdfEntry>& entries) {
    table_->blockSignals(true);
    table_->setColumnCount(3);
    table_->setHorizontalHeaderLabels({"Key", "Type", "Value"});
    table_->setItemDelegateForColumn(1, new LdfTypeDelegate(table_));
    table_->setRowCount(static_cast<int>(entries.size()));
    for (int i = 0; i < static_cast<int>(entries.size()); ++i) {
        auto& e = entries[i];
        table_->setItem(i, 0, new QTableWidgetItem(sanitize(e.key)));
        table_->setItem(i, 1, new QTableWidgetItem(ldf_type_display(static_cast<uint8_t>(e.type))));
        table_->setItem(i, 2, new QTableWidgetItem(sanitize(e.raw_value)));
    }
    table_->blockSignals(false);
}

void LdfTableWidget::set_config(const lu::assets::LdfConfig& config) {
    table_->blockSignals(true);
    table_->setColumnCount(2);
    table_->setHorizontalHeaderLabels({"Key", "Value"});
    table_->setItemDelegateForColumn(1, nullptr);
    table_->setRowCount(static_cast<int>(config.size()));
    for (int i = 0; i < static_cast<int>(config.size()); ++i) {
        table_->setItem(i, 0, new QTableWidgetItem(sanitize(config[i].first)));
        table_->setItem(i, 1, new QTableWidgetItem(sanitize(config[i].second)));
    }
    table_->blockSignals(false);
}

void LdfTableWidget::clear() {
    table_->setRowCount(0);
}

void LdfTableWidget::on_add() {
    int row = table_->rowCount();
    table_->blockSignals(true);
    table_->insertRow(row);
    table_->setItem(row, 0, new QTableWidgetItem("new_key"));
    if (table_->columnCount() == 3) {
        table_->setItem(row, 1, new QTableWidgetItem(ldf_type_display(0)));
        table_->setItem(row, 2, new QTableWidgetItem(""));
    } else {
        table_->setItem(row, 1, new QTableWidgetItem(""));
    }
    table_->blockSignals(false);
    emit entry_added();
}

void LdfTableWidget::on_remove() {
    int row = table_->currentRow();
    if (row < 0) return;
    emit entry_removed(row);
    table_->removeRow(row);
}

} // namespace lu_widgets
