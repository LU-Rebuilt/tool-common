#pragma once

#include "netdevil/zone/luz/luz_types.h"
#include <QWidget>
#include <QDoubleSpinBox>

namespace lu_widgets {

class VecEditWidget : public QWidget {
    Q_OBJECT
public:
    explicit VecEditWidget(int components = 3, QWidget* parent = nullptr);

    void set_vec3(const lu::assets::Vec3& v);
    lu::assets::Vec3 get_vec3() const;
    void set_quat(const lu::assets::Quat& q);
    lu::assets::Quat get_quat() const;

signals:
    void value_changed();

private:
    QDoubleSpinBox* spins_[4]{};
    int components_;
};

} // namespace lu_widgets
