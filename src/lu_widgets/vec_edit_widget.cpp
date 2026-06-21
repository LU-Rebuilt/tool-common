#include "vec_edit_widget.h"
#include <QHBoxLayout>
#include <QLabel>

namespace lu_widgets {

VecEditWidget::VecEditWidget(int components, QWidget* parent)
    : QWidget(parent), components_(components) {
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    const char* labels[] = {"X", "Y", "Z", "W"};
    for (int i = 0; i < components; ++i) {
        layout->addWidget(new QLabel(labels[i]));
        spins_[i] = new QDoubleSpinBox;
        spins_[i]->setDecimals(4);
        spins_[i]->setRange(-1e6, 1e6);
        spins_[i]->setSingleStep(0.1);
        layout->addWidget(spins_[i]);
        connect(spins_[i], &QDoubleSpinBox::editingFinished, this, &VecEditWidget::value_changed);
    }
}

void VecEditWidget::set_vec3(const lu::assets::Vec3& v) {
    spins_[0]->setValue(v.x);
    spins_[1]->setValue(v.y);
    spins_[2]->setValue(v.z);
}

lu::assets::Vec3 VecEditWidget::get_vec3() const {
    return {static_cast<float>(spins_[0]->value()),
            static_cast<float>(spins_[1]->value()),
            static_cast<float>(spins_[2]->value())};
}

void VecEditWidget::set_quat(const lu::assets::Quat& q) {
    spins_[0]->setValue(q.x);
    spins_[1]->setValue(q.y);
    spins_[2]->setValue(q.z);
    if (components_ >= 4) spins_[3]->setValue(q.w);
}

lu::assets::Quat VecEditWidget::get_quat() const {
    lu::assets::Quat q;
    q.x = static_cast<float>(spins_[0]->value());
    q.y = static_cast<float>(spins_[1]->value());
    q.z = static_cast<float>(spins_[2]->value());
    q.w = (components_ >= 4) ? static_cast<float>(spins_[3]->value()) : 1.0f;
    return q;
}

} // namespace lu_widgets
