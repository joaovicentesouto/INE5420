/* The MIT License
 *
 * Copyright (c) 2019 João Vicente Souto and Bruno Izaias Bonotto
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CONTROL_MAIN_CONTROL_HPP
#define CONTROL_MAIN_CONTROL_HPP

/* External includes */
#include <vector>
#include <iostream>
#include <gtkmm.h>

/* local includes */
#include "../model/geometry.hpp"
#include "../model/shape.hpp"
#include "../model/point.hpp"
#include "../model/polygon.hpp"
#include "../model/window.hpp"
#include "../model/viewport.hpp"

namespace control
{

/*================================================================================*/
/*                                   Definitions                                  */
/*================================================================================*/

	class ModelColumns : public Gtk::TreeModel::ColumnRecord
	{
	public:

		ModelColumns()
		{
			add(_column_id);
			add(_column_name);
			add(_column_type);
		}

		Gtk::TreeModelColumn<int> _column_id;
		Gtk::TreeModelColumn<Glib::ustring> _column_name;
		Gtk::TreeModelColumn<Glib::ustring> _column_type;
	};

	class MainControl
	{
	public:

		MainControl(Glib::RefPtr<Gtk::Builder>& builder) :
			_builder(builder)
		{
			build_window();
			build_viewport();
			build_tree_view();
			build_new_objects();
			build_movements();
		}

		~MainControl()
		{
			delete _window;
			delete _viewport;
		}

		void up();
		void left();
		void right();
		void down();
		void zoom_in();
		void zoom_out();

		void on_item_selected();
		void on_radio_clicked();
		void on_new_object_clicked();
		void on_dialog_exit_clicked();
		void on_dialog_ok_clicked();

	private:
		void build_window();
		void build_viewport();
		void build_tree_view();
		void build_new_objects();
		void build_movements();

		void insert_point(std::string name);
		void insert_polygon(std::string name);
		void insert_object(std::string name, std::string type);

		void add_entry(int id, std::string name, std::string type);

		/* Control */
		int _row_selected{0};
		int _objects_control{0};

		/* Model */
		model::Window   *_window{nullptr};
		model::Viewport *_viewport{nullptr};

		/* Shapes */
		std::vector<std::shared_ptr<model::Shape>> _shapes;
		std::map<int, std::shared_ptr<model::Shape>> _shapes_map;

		/* Gtk */
		ModelColumns _tree_model;
		Glib::RefPtr<Gtk::Builder> _builder;
		Glib::RefPtr<Gtk::ListStore> _ref_tree_model;
		Glib::RefPtr<Gtk::TreeSelection> _tree_selection;
	};

/*================================================================================*/
/*                                 Implementaions                                 */
/*================================================================================*/

	void MainControl::build_window()
	{
		Gtk::DrawingArea *draw;
		Gtk::Allocation alloc;
		double width, height;

		_builder->get_widget("area_draw", draw);

		draw->set_hexpand(true);
		alloc = draw->get_allocation();
		width = alloc.get_width();
		height = alloc.get_height();

		_window = new model::Window(model::Vector(0, 0), model::Vector(width, height), *draw);
	}

	void MainControl::build_viewport()
	{
		Gtk::DrawingArea *draw;
		_builder->get_widget("area_draw", draw);
		_viewport = new model::Viewport(*_window, _shapes, *draw);
	}

	void MainControl::build_tree_view()
	{
		Gtk::TreeView * tree;

		_builder->get_widget("tree_objects", tree);
		_ref_tree_model = Gtk::ListStore::create(_tree_model);

		tree->set_model(_ref_tree_model);
		tree->append_column("ID", _tree_model._column_id);
		tree->append_column("Name", _tree_model._column_name);
		tree->append_column("Type", _tree_model._column_type);

		_tree_selection = tree->get_selection();
		_tree_selection->signal_changed().connect(sigc::mem_fun(*this, &MainControl::on_item_selected));

		Gtk::TreeModel::Row row         = *(_ref_tree_model->append());
		row[_tree_model._column_id]     = 0;
		row[_tree_model._column_name]   = "Window";
		row[_tree_model._column_type]   = "View";
	}

	void MainControl::build_new_objects()
	{
		Gtk::Button* button;
		Gtk::RadioButton* radio1;
		Gtk::RadioButton* radio2;

		_builder->get_widget("button_new_object", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_new_object_clicked));

		_builder->get_widget("dialog_button_cancel", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_dialog_exit_clicked));

		_builder->get_widget("dialog_button_ok", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_dialog_ok_clicked));

		_builder->get_widget("radio_point", radio1);
		radio1->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_radio_clicked));

		_builder->get_widget("radio_line", radio2);
		radio2->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_radio_clicked));
		radio2->join_group(*radio1);

		_builder->get_widget("radio_rectangle", radio2);
		radio2->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_radio_clicked));
		radio2->join_group(*radio1);

		_builder->get_widget("radio_polygon", radio2);
		radio2->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_radio_clicked));
		radio2->join_group(*radio1);
	}

	void MainControl::on_item_selected()
	{
		_row_selected = _tree_selection->get_selected()->get_value(_tree_model._column_id);
	}

	void MainControl::build_movements()
	{
		Gtk::SpinButton *spin;

		_builder->get_widget("spin_step", spin);

		spin->set_digits(5);
		spin->set_range(0, 9999999);
		spin->set_increments(1, 100);
		spin->set_value(2);

		Gtk::Button *btn;
		_builder->get_widget("button_up", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::up));

		_builder->get_widget("button_left", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::left));

		_builder->get_widget("button_right", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::right));

		_builder->get_widget("button_down", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::down));

		_builder->get_widget("button_in", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::zoom_in));

		_builder->get_widget("button_out", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::zoom_out));
	}

	void MainControl::on_new_object_clicked()
	{
		Gtk::Dialog* dialog;
		_builder->get_widget("window_dialog", dialog);
  		dialog->run();
	}

	void MainControl::on_radio_clicked()
	{
		// std::cout << "AI" << std::endl;
	}

	void MainControl::on_dialog_exit_clicked()
	{
		Gtk::Dialog* dialog;
		_builder->get_widget("window_dialog", dialog);
  		dialog->close();
	}

	void MainControl::on_dialog_ok_clicked()
	{
		Gtk::RadioButton* radio;
		Gtk::Entry* entry;

		_builder->get_widget("radio_point", radio);
		_builder->get_widget("entry_name", entry);

  		std::string name = std::string(entry->get_text());

  		if (radio->get_active()) {
  			insert_point(name);
  		}
  		else
  		{
			_builder->get_widget("radio_line", radio);

			if (radio->get_active()) {
	  			insert_object(name, "Line");
			}
	  		else
	  		{
				_builder->get_widget("radio_rectangle", radio);

				if (radio->get_active())
		  			insert_object(name, "Rectangle");
		  		else
		  			insert_polygon(name);
	  		}
  		}

		_viewport->update();
	}

	void MainControl::insert_point(std::string name)
	{
		Gtk::Entry* entry;
		double x, y, z;

		_builder->get_widget("entry_point_x", entry);
		x = atof(std::string(entry->get_text()).c_str());

		_builder->get_widget("entry_point_y", entry);
		y = atof(std::string(entry->get_text()).c_str());

		if (model::Vector::dimension == 4)
		{
			_builder->get_widget("entry_point_z", entry);
			z = atof(std::string(entry->get_text()).c_str());
		}
		else
			z = model::Vector::z;

		add_entry(++_objects_control, name, "Point");

		_shapes.emplace_back(new model::Point(name, x, y, z));

		_shapes_map[_objects_control] = _shapes.back();
	}

	void MainControl::insert_object(std::string name, std::string type)
	{
		Gtk::Entry* entry;
		double x1, y1, z1, x2, y2, z2;

		_builder->get_widget("entry_x1_" + type, entry);
		x1 = atof(std::string(entry->get_text()).c_str());

		_builder->get_widget("entry_y1_" + type, entry);
		y1 = atof(std::string(entry->get_text()).c_str());

		_builder->get_widget("entry_x2_" + type, entry);
		x2 = atof(std::string(entry->get_text()).c_str());

		_builder->get_widget("entry_y2_" + type, entry);
		y2 = atof(std::string(entry->get_text()).c_str());

		if (model::Vector::dimension == 4)
		{
			_builder->get_widget("entry_z1_" + type, entry);
			z1 = atof(std::string(entry->get_text()).c_str());

			_builder->get_widget("entry_z2_" + type, entry);
			z2 = atof(std::string(entry->get_text()).c_str());
		}
		else
			z1 = z2 = model::Vector::z;

		add_entry(++_objects_control, name, type);

		if (!type.compare("Line"))
			_shapes.emplace_back(new model::Line(name, model::Vector(x1, y1, z1), model::Vector(x2, y2, z2)));
		else
			_shapes.emplace_back(new model::Rectangle(name, model::Vector(x1, y1, z1), model::Vector(x2, y2, z2)));

		_shapes_map[_objects_control] = _shapes.back();
	}

	void MainControl::insert_polygon(std::string name)
	{
		// Gtk::Box* box;
		// Gtk::Entry* entry;
		// int num_vectors;

		// _builder->get_widget("entry_points_number", entry);
		// num_vectors = atoi(std::string(entry->get_text()).c_str());

		// _builder->get_widget("entry_points_number", entry);
	}

	void MainControl::add_entry(int id, std::string name, std::string type)
	{
			Gtk::TreeModel::Row row = *(_ref_tree_model->append());
			row[_tree_model._column_id]   = id;
			row[_tree_model._column_name] = name;
			row[_tree_model._column_type] = type;
	}

	void MainControl::up()
	{
		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = spin->get_value();

		auto T = model::transformations::translation(model::Vector(0, step));

		if (!_row_selected)
			_window->transformation(T);
		else
			_shapes_map[_row_selected]->transformation(T);

		_viewport->update();
	}

	void MainControl::left()
	{
		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = spin->get_value();

		auto T = model::transformations::translation(model::Vector(-step, 0));

		if (!_row_selected)
			_window->transformation(T);
		else
			_shapes_map[_row_selected]->transformation(T);

		_viewport->update();
	}

	void MainControl::right()
	{
		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = spin->get_value();

		auto T = model::transformations::translation(model::Vector(step, 0));

		if (!_row_selected)
			_window->transformation(T);
		else
			_shapes_map[_row_selected]->transformation(T);

		_viewport->update();
	}

	void MainControl::down()
	{
		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = spin->get_value();

		auto T = model::transformations::translation(model::Vector(0, -step));

		if (!_row_selected)
			_window->transformation(T);
		else
			_shapes_map[_row_selected]->transformation(T);

		_viewport->update();
	}

	void MainControl::zoom_in()
	{
		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = spin->get_value();

		if (!_row_selected)
		{
			auto mass_center = _window->mass_center();
			auto T = model::transformations::scheduling(step, mass_center);
			_window->transformation(T);
		}
		else
		{
			auto mass_center = _shapes_map[_row_selected]->mass_center();
			auto T = model::transformations::scheduling(step, mass_center);
			_shapes_map[_row_selected]->transformation(T);
		}

		_viewport->update();
	}

	void MainControl::zoom_out()
	{
		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = spin->get_value();

		if (!_row_selected)
		{
			auto mass_center = _window->mass_center();
			auto T = model::transformations::scheduling(1/step, mass_center);
			_window->transformation(T);
		}
		else
		{
			auto mass_center = _shapes_map[_row_selected]->mass_center();
			auto T = model::transformations::scheduling(1/step, mass_center);
			_shapes_map[_row_selected]->transformation(T);
		}

		_viewport->update();
	}


} //! namespace control

#endif  // CONTROL_MAIN_CONTROL_HPP