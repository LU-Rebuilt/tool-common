#pragma once

#include "netdevil/common/ldf/ldf_types.h"
#include <QWidget>
#include <QTableWidget>
#include <QPushButton>

namespace lu_widgets {

class LdfTableWidget : public QWidget {
    Q_OBJECT
public:
    explicit LdfTableWidget(QWidget* parent = nullptr);

    void set_entries(const std::vector<lu::assets::LdfEntry>& entries);
    void set_config(const lu::assets::LdfConfig& config);
    void clear();

signals:
    void entry_added();
    void entry_removed(int row);
    void entry_edited(int row, const QString& key, const QString& raw_value);

private slots:
    void on_add();
    void on_remove();

private:
    QTableWidget* table_;
    QPushButton* add_btn_;
    QPushButton* remove_btn_;
};

} // namespace lu_widgets
