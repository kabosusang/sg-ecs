#pragma once
#include <cstddef>
#include <flat_map>
#include <vector>

#include "blob_array.hpp"
#include "core/component.hpp"
#include "core/entity.hpp"
#include "core/component_registry.hpp"


namespace ecs {
class table {
public:
	std::vector<entity> entities_;
	std::vector<blob_array> columns_;
	std::vector<component_id_type> col_component_ids_;
	std::flat_map<component_id_type, size_t> comp_to_col_;

	[[nodiscard]] size_t row_count() const noexcept { return entities_.size(); }

	template <typename T>
	[[nodiscard]] blob_array& column_blob() {
		return columns_[comp_to_col_.find(type_index<T>())->second];
	}

	template <typename T>
	[[nodiscard]] T& at(size_t row) noexcept {
		return blob_array_accessor<T>::get(column_blob<T>(), row);
	}

	template <typename T>
	[[nodiscard]] T* column_ptr() noexcept {
		auto it = comp_to_col_.find(type_index<T>());
		if (it == comp_to_col_.end()) {
			return nullptr;
		}
		return blob_array_accessor<T>::ptr(columns_[it->second]);
	}

	void ensure_column_by_id(component_id_type cid, const component_registry& registry) {
		if (comp_to_col_.contains(cid)) {
			return;
		}
		comp_to_col_[cid] = columns_.size();
		col_component_ids_.push_back(cid);
		columns_.push_back(registry.make_column(cid));
		columns_.back().resize(entities_.size());
	}

	template <typename T>
	void ensure_column(const component_registry& registry) {
		ensure_column_by_id(type_index<T>(), registry);
	}

	size_t push_row(entity e) {
		size_t row = entities_.size();
		entities_.push_back(e);
		for (auto& col : columns_) {
			col.resize(entities_.size());
		}
		return row;
	}

	template <typename OnMove>
	void swap_remove(size_t row, OnMove&& on_move) {
		auto last = entities_.size() - 1;

		if (row != last) {
			auto moved = entities_[last]; // 最后一个实体
            
			for (auto& col : columns_) {
				col.copy_from(col, last, row);
			} // 最后一行数据复制到删除位置
            
			entities_[row] = moved;
			on_move(moved, row);
		}
		for (auto& col : columns_) {
			col.swap_remove(last);
		}//删除最后一列的组件数据
        
        //删除最后一个实体
		entities_.pop_back();
	}

	void copy_column_from(component_id_type cid, const table& src, size_t src_row, size_t dst_row) {
		auto sit = src.comp_to_col_.find(cid);
		auto dit = comp_to_col_.find(cid);
		if (sit == src.comp_to_col_.end() || dit == comp_to_col_.end()) {
			return;
		}
		columns_[dit->second].copy_from(src.columns_[sit->second], src_row, dst_row);
	}
};

} //namespace sago::ecs











