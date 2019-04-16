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
#include <unordered_map>
#include <vector>
#include <gtkmm.h>

/* local includes */
#include "../config/traits.hpp"
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

	class ModelColumnsObjects : public Gtk::TreeModel::ColumnRecord
	{
	public:

		ModelColumnsObjects()
		{
			add(_column_id);
			add(_column_name);
			add(_column_type);
		}

		Gtk::TreeModelColumn<int> _column_id;
		Gtk::TreeModelColumn<Glib::ustring> _column_name;
		Gtk::TreeModelColumn<Glib::ustring> _column_type;
	};

	class ModelColumnsVectors : public Gtk::TreeModel::ColumnRecord
	{
	public:

		ModelColumnsVectors()
		{
			add(_column_id);
			add(_column_x);
			add(_column_y);
			add(_column_z);
		}

		Gtk::TreeModelColumn<int> _column_id;
		Gtk::TreeModelColumn<double> _column_x;
		Gtk::TreeModelColumn<double> _column_y;
		Gtk::TreeModelColumn<double> _column_z;
	};

	class MainControl
	{
	private:
		enum ButtonID
		{
			/* Objects */
			Null           = 0,
			Point          = 1,
			Line           = 2,
			Rectangle      = 3,
			Polygon        = 4,
			/* Rotation */
			CenterObject   = 5,
			CenterWorld    = 6,
			CenterSpecific = 7
		};

	public:

		MainControl(Glib::RefPtr<Gtk::Builder>& builder) :
			_builder(builder)
		{
			build_window();
			build_viewport();
			build_tree_views();
			build_new_objects();
			build_movements();
			build_numeric_entrys();

			if (model::Vector::dimension == 3)
				disable_unused_interface_objects(ButtonID::Null);
		}

		~MainControl()
		{
			delete _viewport;
		}

		void up();
		void left();
		void right();
		void down();
		void zoom_in();
		void zoom_out();
		void clockwise();
		void counterclockwise();

		void on_vector_selected();
		void on_object_selected();
		void on_radio_clicked();
		void on_dialog_ok_clicked();
		void on_new_object_clicked();
		void on_dialog_exit_clicked();
		void on_remove_object_clicked();
		void on_dialog_insert_clicked();
		void on_dialog_delete_clicked();

	private:
		void build_window();
		void build_viewport();
		void build_tree_views();
		void build_new_objects();
		void build_movements();
		void build_numeric_entrys();
		void reset_dialog_entries();
		void build_objects(std::vector<std::shared_ptr<model::Shape>> shapes);
		void enable_used_interface_objects(ButtonID selected);
		void disable_unused_interface_objects(ButtonID selected);

		void insert_point(std::string name);
		void insert_polygon(std::string name);
		void insert_object(std::string name, std::string type);

		void erase_object_entry(int id);
		void erase_vector_entry(int id);
		void add_entry(int id, double x, double y, double z);
		void add_entry(int id, std::string name, std::string type);

		/* Control */
		int _vector_selected{-1};
		int _shape_selected{0};
		int _objects_control{0};
		int _vectors_control{0};

		/* Model */
		model::Window   *_window{nullptr};   //! Associate with shared_ptr
		model::Viewport *_viewport{nullptr};

		/* Shapes */
		std::vector<std::shared_ptr<model::Shape>> _shapes;
		std::unordered_map<int, std::shared_ptr<model::Shape>> _shapes_map;

		/* Gtk */
		Glib::RefPtr<Gtk::Builder> _builder;
		ModelColumnsObjects _tree_model_objects;
		ModelColumnsVectors _tree_model_vectors;
		Glib::RefPtr<Gtk::ListStore> _list_model_objects;
		Glib::RefPtr<Gtk::ListStore> _list_model_vectors;
	};

/*================================================================================*/
/*                                 Implementaions                                 */
/*================================================================================*/

	void MainControl::build_window()
	{
		db<MainControl>(TRC) << "Build window" << std::endl;

		Gtk::DrawingArea *draw;
		Gtk::Allocation alloc;
		double width{0}, height{0};

		_builder->get_widget("area_draw", draw);

		draw->set_hexpand(true);
		alloc = draw->get_allocation();
		width = alloc.get_width() / 2;
		height = alloc.get_height() / 2;

		_window = new model::Window(model::Vector(-width + 10, -height + 10), model::Vector(width - 10, height - 10));

		_shapes.push_back(std::make_shared<model::Shape>(_window->drawable()));
		_shapes_map[_objects_control++] = _shapes.back();
	}

	void MainControl::build_viewport()
	{
		db<MainControl>(TRC) << "Build viewport" << std::endl;

		Gtk::DrawingArea *draw;
		_builder->get_widget("area_draw", draw);
		_viewport = new model::Viewport(*_window, _shapes, *draw);
	}

	void MainControl::build_tree_views()
	{
		db<MainControl>(TRC) << "Build tree views" << std::endl;

		Gtk::TreeView *tree;

		_builder->get_widget("tree_objects", tree);
		_list_model_objects = Gtk::ListStore::create(_tree_model_objects);

		tree->set_model(_list_model_objects);
		tree->append_column("ID", _tree_model_objects._column_id);
		tree->append_column("Name", _tree_model_objects._column_name);
		tree->append_column("Type", _tree_model_objects._column_type);

        tree->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &MainControl::on_object_selected));

		Gtk::TreeModel::Row row = *(_list_model_objects->append());
		row[_tree_model_objects._column_id]   = 0;
		row[_tree_model_objects._column_name] = "Window";
		row[_tree_model_objects._column_type] = "View";

		_builder->get_widget("tree_vectors", tree);
		_list_model_vectors = Gtk::ListStore::create(_tree_model_vectors);

		tree->set_model(_list_model_vectors);
		tree->append_column("ID", _tree_model_vectors._column_id);
		tree->append_column("X", _tree_model_vectors._column_x);
		tree->append_column("Y", _tree_model_vectors._column_y);
		tree->append_column("Z", _tree_model_vectors._column_z);

        tree->get_selection()->signal_changed().connect(sigc::mem_fun(*this, &MainControl::on_vector_selected));
	}

	void MainControl::build_new_objects()
	{
		db<MainControl>(TRC) << "Build buttons" << std::endl;

		Gtk::Button      *button;
		Gtk::RadioButton *radio1;
		Gtk::RadioButton *radio2;

		_builder->get_widget("button_new_object", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_new_object_clicked));

		_builder->get_widget("dialog_button_cancel", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_dialog_exit_clicked));

		_builder->get_widget("dialog_button_ok", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_dialog_ok_clicked));

		_builder->get_widget("button_insert_vector", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_dialog_insert_clicked));

		_builder->get_widget("button_remove_object", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_remove_object_clicked));

		_builder->get_widget("button_delete_vector", button);
		button->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_dialog_delete_clicked));

		/* New object group */
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

		/* Rotation group */
		_builder->get_widget("radio_center_object", radio1);
		radio1->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_radio_clicked));

		_builder->get_widget("radio_center_world", radio2);
		radio2->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_radio_clicked));
		radio2->join_group(*radio1);

		_builder->get_widget("radio_center_specific", radio2);
		radio2->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::on_radio_clicked));
		radio2->join_group(*radio1);
	}

	void MainControl::on_object_selected()
	{
		db<MainControl>(TRC) << "MainControl::on_object_selected()" << std::endl;

		Gtk::TreeView *tree;
		_builder->get_widget("tree_objects", tree);

		_shape_selected = tree->get_selection()->get_selected()->get_value(_tree_model_objects._column_id);

		db<MainControl>(INF) << "Shape selected: " << _shape_selected << std::endl;
	}

	void MainControl::on_vector_selected()
	{
		db<MainControl>(TRC) << "MainControl::on_vector_selected()" << std::endl;

		Gtk::TreeView *tree;
		_builder->get_widget("tree_vectors", tree);

		_vector_selected = tree->get_selection()->get_selected()->get_value(_tree_model_vectors._column_id);

		db<MainControl>(INF) << "Vector selected: " << _vector_selected << std::endl;
	}

	void MainControl::build_movements()
	{
		db<MainControl>(TRC) << "Build movements" << std::endl;

		Gtk::SpinButton *spin;

		_builder->get_widget("spin_step", spin);

		spin->set_range(0, 9999999);
		spin->set_increments(1, 100);
		spin->set_value(0);

		_builder->get_widget("spin_percentual", spin);

		spin->set_range(1, 9999999);
		spin->set_increments(1, 100);
		spin->set_value(1);

		_builder->get_widget("spin_degrees", spin);

		spin->set_range(0, 9999999);
		spin->set_increments(1, 100);
		spin->set_value(0);

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

		_builder->get_widget("button_r_left", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::counterclockwise));

		_builder->get_widget("button_r_right", btn);
		btn->signal_clicked().connect(sigc::mem_fun(*this, &MainControl::clockwise));
	}

	void MainControl::build_numeric_entrys()
	{
		db<MainControl>(TRC) << "Build numeric entrys" << std::endl;

		Gtk::Entry *entry;
		std::vector<std::string> entry_names{
			"entry_point_x", "entry_point_y", "entry_point_z",
			"entry_x1_Line", "entry_y1_Line", "entry_z1_Line",
			"entry_x2_Line", "entry_y2_Line", "entry_z2_Line",
			"entry_x1_Rectangle", "entry_y1_Rectangle", "entry_z1_Rectangle",
			"entry_x2_Rectangle", "entry_y2_Rectangle", "entry_z2_Rectangle",
			"entry_polygon_x", "entry_polygon_y", "entry_polygon_z"
		};

		((void) entry);       //! Suppress unused variable warning.
		((void) entry_names); //! Suppress unused variable warning.

		//! Not working

		// for (auto name : entry_names)
		// {
		// 	_builder->get_widget(name, entry);
		// 	entry->signal_insert_text().connect(
		// 		[entry](const Glib::ustring &text, int *position)
		// 		{
		// 			bool only_number = text.length() != 0;

		// 			for (unsigned int i = 0; i < text.length(); i++)
		// 				only_number &= Glib::Unicode::isdigit(text[i]);
					
		// 			std::string old = entry->get_text();

		// 			if (old.size() <= *position)
		// 				old += text;
		// 			else
		// 				old[*position] = text[0];
					
		// 			if (only_number)
		// 				entry->set_text(old);
		// 		}
		// 	);
		// }
	}

	void MainControl::disable_unused_interface_objects(ButtonID selected)
	{
		db<MainControl>(TRC) << "Disable unused interface objects" << std::endl;

		std::vector<std::string> entry_names;
		std::vector<std::string> button_names;

		switch(selected)
		{
			case ButtonID::Null:
				entry_names = {
					"entry_point_z",
					"entry_x1_Line",
					"entry_x2_Line",
					"entry_y1_Line",
					"entry_y2_Line",
					"entry_z1_Line",
					"entry_z2_Line",
					"entry_x1_Rectangle",
					"entry_x2_Rectangle",
					"entry_y1_Rectangle",
					"entry_y2_Rectangle",
					"entry_z1_Rectangle",
					"entry_z2_Rectangle",
					"entry_polygon_x",
					"entry_polygon_y",
					"entry_polygon_z",
					"entry_rotation_x",
					"entry_rotation_y",
					"entry_rotation_z"
				};
				button_names = {
					"button_insert_vector",
					"button_delete_vector"
				};
				break;

			case ButtonID::Point:
				entry_names = {
					"entry_x1_Line",
					"entry_x2_Line",
					"entry_y1_Line",
					"entry_y2_Line",
					"entry_x1_Rectangle",
					"entry_x2_Rectangle",
					"entry_y1_Rectangle",
					"entry_y2_Rectangle",
					"entry_polygon_x",
					"entry_polygon_y",
					"entry_polygon_z"
				};
				button_names = {
					"button_insert_vector",
					"button_delete_vector"
				};
				break;

			case ButtonID::Line:
				entry_names = {
					"entry_point_x",
					"entry_point_y",
					"entry_x1_Rectangle",
					"entry_x2_Rectangle",
					"entry_y1_Rectangle",
					"entry_y2_Rectangle",
					"entry_polygon_x",
					"entry_polygon_y",
					"entry_polygon_z"
				};
				button_names = {
					"button_insert_vector",
					"button_delete_vector"
				};
				break;

			case ButtonID::Rectangle:
				entry_names = {
					"entry_point_x",
					"entry_point_y",
					"entry_x1_Line",
					"entry_x2_Line",
					"entry_y1_Line",
					"entry_y2_Line",
					"entry_polygon_x",
					"entry_polygon_y",
					"entry_polygon_z"
				};
				button_names = {
					"button_insert_vector",
					"button_delete_vector"
				};
				break;

			case ButtonID::Polygon:
				entry_names = {
					"entry_point_x",
					"entry_point_y",
					"entry_x1_Line",
					"entry_x2_Line",
					"entry_y1_Line",
					"entry_y2_Line",
					"entry_x1_Rectangle",
					"entry_x2_Rectangle",
					"entry_y1_Rectangle",
					"entry_y2_Rectangle"
				};
				break;

			case ButtonID::CenterObject:
				entry_names = {
					"entry_rotation_x",
					"entry_rotation_y"
				};
				break;

			case ButtonID::CenterWorld:
				entry_names = {
					"entry_rotation_x",
					"entry_rotation_y"
				};
				break;

			default:
				break;
		}
		
		Gtk::Entry *entry;
		Gtk::Button *button;

		for (auto name : entry_names)
		{
			_builder->get_widget(name, entry);
			entry->set_sensitive(false);
		};

		for (auto name : button_names)
		{
			_builder->get_widget(name, button);
			button->set_sensitive(false);
		}
	}

	void MainControl::enable_used_interface_objects(ButtonID selected)
	{
		db<MainControl>(TRC) << "Enable used interface objects" << std::endl;

		std::vector<std::string> entry_names;
		std::vector<std::string> button_names;

		switch(selected)
		{
			case ButtonID::Point:
				entry_names = {
					"entry_point_x",
					"entry_point_y"
				};
				break;

			case ButtonID::Line:
				entry_names = {
					"entry_x1_Line",
					"entry_x2_Line",
					"entry_y1_Line",
					"entry_y2_Line"
				};
				break;

			case ButtonID::Rectangle:
				entry_names = {
					"entry_x1_Rectangle",
					"entry_x2_Rectangle",
					"entry_y1_Rectangle",
					"entry_y2_Rectangle"
				};
				break;

			case ButtonID::Polygon:
				entry_names = {
					"entry_polygon_x",
					"entry_polygon_y"
				};
				button_names = {
					"button_insert_vector",
					"button_delete_vector"
				};
				break;

			case ButtonID::CenterSpecific:
				entry_names = {
					"entry_rotation_x",
					"entry_rotation_y"
				};
				break;

			default:
				break;
		}
		
		Gtk::Entry *entry;
		Gtk::Button *button;

		for (auto name : entry_names)
		{
			_builder->get_widget(name, entry);
			entry->set_sensitive(true);
		};

		for (auto name : button_names)
		{
			_builder->get_widget(name, button);
			button->set_sensitive(true);
		}
	}

	void MainControl::on_new_object_clicked()
	{
		db<MainControl>(TRC) << "MainControl::on_new_object_clicked()" << std::endl;

		Gtk::Dialog *dialog;
		_builder->get_widget("window_dialog", dialog);
  		dialog->run();
	}

	void MainControl::on_radio_clicked()
	{
		db<MainControl>(TRC) << "MainControl::on_radio_clicked()" << std::endl;
		
		Gtk::RadioButton *radio;

		/* Object type */
		unsigned hash = 0;

		_builder->get_widget("radio_point", radio);
		hash += radio->get_active() ? ButtonID::Point     	   : ButtonID::Null;

		_builder->get_widget("radio_line", radio);
		hash += radio->get_active() ? ButtonID::Line      	   : ButtonID::Null;

		_builder->get_widget("radio_rectangle", radio);
		hash += radio->get_active() ? ButtonID::Rectangle 	   : ButtonID::Null;

		_builder->get_widget("radio_polygon", radio);
		hash += radio->get_active() ? ButtonID::Polygon   	   : ButtonID::Null;

		switch (hash)
		{
			case ButtonID::Point:
				disable_unused_interface_objects(ButtonID::Point);
				enable_used_interface_objects(ButtonID::Point);
				break;

			case ButtonID::Line:
				disable_unused_interface_objects(ButtonID::Line);
				enable_used_interface_objects(ButtonID::Line);
				break;

			case ButtonID::Rectangle:
				disable_unused_interface_objects(ButtonID::Rectangle);
				enable_used_interface_objects(ButtonID::Rectangle);
				break;

			case ButtonID::Polygon:
				disable_unused_interface_objects(ButtonID::Polygon);
				enable_used_interface_objects(ButtonID::Polygon);
				break;

			/* Undefined */
			default:
				break;
		}

		hash = 0;

		_builder->get_widget("radio_center_object", radio);
		hash += radio->get_active() ? ButtonID::CenterObject   : ButtonID::Null;

		_builder->get_widget("radio_center_world", radio);
		hash += radio->get_active() ? ButtonID::CenterWorld    : ButtonID::Null;

		_builder->get_widget("radio_center_specific", radio);
		hash += radio->get_active() ? ButtonID::CenterSpecific : ButtonID::Null;

		switch (hash)
		{
			case ButtonID::CenterObject:
				disable_unused_interface_objects(ButtonID::CenterObject);
				enable_used_interface_objects(ButtonID::CenterObject);
				break;

			case ButtonID::CenterWorld:
				disable_unused_interface_objects(ButtonID::CenterWorld);
				enable_used_interface_objects(ButtonID::CenterWorld);
				break;

			case ButtonID::CenterSpecific:
				disable_unused_interface_objects(ButtonID::CenterSpecific);
				enable_used_interface_objects(ButtonID::CenterSpecific);
				break;

			/* Undefined */
			default:
				break;
		}
	}

	void MainControl::on_dialog_exit_clicked()
	{
		db<MainControl>(TRC) << "MainControl::on_dialog_exit_clicked()" << std::endl;

		Gtk::Dialog *dialog;
		_builder->get_widget("window_dialog", dialog);
  		dialog->hide();

		reset_dialog_entries();
	}

	void MainControl::reset_dialog_entries()
	{
		std::vector<std::string> entry_names = {
			"entry_name",
			"entry_point_x",
			"entry_point_y",
			"entry_x1_Line",
			"entry_x2_Line",
			"entry_y1_Line",
			"entry_y2_Line",
			"entry_x1_Rectangle",
			"entry_x2_Rectangle",
			"entry_y1_Rectangle",
			"entry_y2_Rectangle",
			"entry_polygon_x",
			"entry_polygon_y",
			"entry_polygon_z"
		};
		
		Gtk::Entry *entry;

		for (auto name : entry_names)
		{
			_builder->get_widget(name, entry);
			entry->set_text("");
		};

		_list_model_vectors->clear();
		_vector_selected = -1;
		_vectors_control = 0;
	}

	void MainControl::on_dialog_ok_clicked()
	{
		db<MainControl>(TRC) << "MainControl::on_dialog_ok_clicked()" << std::endl;

		Gtk::Entry       *entry;
		Gtk::RadioButton *radio;
		
		/* Entry name */
		_builder->get_widget("entry_name", entry);
  		std::string name(entry->get_text());

		/* Object type */
		unsigned hash = 0;

		_builder->get_widget("radio_point", radio);
		hash += radio->get_active() ? ButtonID::Point     : ButtonID::Null;

		_builder->get_widget("radio_line", radio);
		hash += radio->get_active() ? ButtonID::Line      : ButtonID::Null;

		_builder->get_widget("radio_rectangle", radio);
		hash += radio->get_active() ? ButtonID::Rectangle : ButtonID::Null;

		_builder->get_widget("radio_polygon", radio);
		hash += radio->get_active() ? ButtonID::Polygon   : ButtonID::Null;

		switch (hash)
		{
			case ButtonID::Point:
				insert_point(name);
				break;

			case ButtonID::Line:
				insert_object(name, "Line");
				break;

			case ButtonID::Rectangle:
				insert_object(name, "Rectangle");
				break;
			
			case ButtonID::Polygon:
				insert_polygon(name);
				break;

			/* Undefined */
			default:
				break;
		}

		_viewport->update();

		reset_dialog_entries();
	}

	void MainControl::on_dialog_insert_clicked()
	{
		db<MainControl>(TRC) << "MainControl::on_dialog_insert_clicked()" << std::endl;

		Gtk::Entry *entry;
		double x{0}, y{0}, z{0};

		_builder->get_widget("entry_polygon_x", entry);
		x = atof(std::string(entry->get_text()).c_str());

		_builder->get_widget("entry_polygon_y", entry);
		y = atof(std::string(entry->get_text()).c_str());

		if (model::Vector::dimension == 4)
		{
			_builder->get_widget("entry_polygon_z", entry);
			z = atof(std::string(entry->get_text()).c_str());
		}
		else
			z = model::Vector::z;

		add_entry(_vectors_control++, x, y, z);
	}

	void MainControl::on_remove_object_clicked()
	{
		if(!_shape_selected)
			return;

		for (auto v = _shapes.begin(); v != _shapes.end(); v++)
			if (*v == _shapes_map[_shape_selected]){
				_shapes.erase(v);
				break;
			}

		_shapes_map.erase(_shape_selected);

		erase_object_entry(_shape_selected);

		_shape_selected = 0;
	}

	void MainControl::on_dialog_delete_clicked()
	{
		if(_vector_selected < 0)
			return;

		erase_vector_entry(_vector_selected);

		_vector_selected = -1;
	}

	void MainControl::build_objects(std::vector<std::shared_ptr<model::Shape>> shapes)
	{
		for (auto shape: shapes)
			if (shape->name().compare("window"))
				shape->transformation(_window->transformation(), _window->normalization());
		
		//clipping
		//viewport transformation
		//draw
	}

	void MainControl::insert_point(std::string name)
	{
		db<MainControl>(TRC) << "MainControl::insert_point()" << std::endl;

		Gtk::Entry *entry;
		double x{0}, y{0}, z{0};

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

		add_entry(_objects_control, name, "Point");

		_shapes.emplace_back(new model::Point(name, x, y, z));
		_shapes_map[_objects_control++] = _shapes.back();

		build_objects({_shapes.back()});
	}

	void MainControl::insert_object(std::string name, std::string type)
	{
		db<MainControl>(TRC) << "MainControl::insert_object()" << std::endl;

		Gtk::Entry *entry;
		double x1{0}, y1{0}, z1{model::Vector::z}, x2{0}, y2{0}, z2{model::Vector::z};

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

		add_entry(_objects_control, name, type);

		if (!type.compare("Line"))
			_shapes.emplace_back(new model::Line(name, model::Vector(x1, y1, z1), model::Vector(x2, y2, z2)));
		else
			_shapes.emplace_back(new model::Rectangle(name, model::Vector(x1, y1, z1), model::Vector(x2, y2, z2)));

		_shapes_map[_objects_control++] = _shapes.back();

		build_objects({_shapes.back()});
	}

	void MainControl::insert_polygon(std::string name)
	{
		db<MainControl>(TRC) << "MainControl::insert_polygon()" << std::endl;

		Gtk::TreeView *tree;
		std::vector<model::Vector> vectors;

		_builder->get_widget("tree_vectors", tree);

		for (auto row : _list_model_vectors->children())
		{
			if (model::Vector::dimension == 4)
		  		vectors.push_back(model::Vector(
					row[_tree_model_vectors._column_x],
					row[_tree_model_vectors._column_y],
					row[_tree_model_vectors._column_z]
				));
			else
		  		vectors.push_back(model::Vector(
					row[_tree_model_vectors._column_x],
					row[_tree_model_vectors._column_y]
				));
		}

		add_entry(_objects_control, name, "Polygon");

		_shapes.emplace_back(new model::Polygon(name, vectors));
		_shapes_map[_objects_control++] = _shapes.back();

		build_objects({_shapes.back()});
	}

	void MainControl::add_entry(int id, std::string name, std::string type)
	{
		db<MainControl>(TRC) << "MainControl::add_entry() - object" << std::endl;
	
		Gtk::TreeModel::Row row = *(_list_model_objects->append());
		row[_tree_model_objects._column_id]   = id;
		row[_tree_model_objects._column_name] = name;
		row[_tree_model_objects._column_type] = type;
	}

	void MainControl::add_entry(int id, double x, double y, double z)
	{
		db<MainControl>(TRC) << "MainControl::add_entry() - vector" << std::endl;
	
		Gtk::TreeModel::Row row = *(_list_model_vectors->append());
		row[_tree_model_vectors._column_id]   = id;
		row[_tree_model_vectors._column_x] = x;
		row[_tree_model_vectors._column_y] = y;
		row[_tree_model_vectors._column_z] = z;
	}

	void MainControl::erase_object_entry(int id)
	{
		db<MainControl>(TRC) << "MainControl::erase_entry() - objects" << std::endl;

		for (auto it: _list_model_objects->children())
		{
			Gtk::TreeModel::Row row = *it;

			if (row[_tree_model_objects._column_id] == id)
			{
				_list_model_objects->erase(it);
				break;
			}
		}
	}

	void MainControl::erase_vector_entry(int id)
	{
		db<MainControl>(TRC) << "MainControl::erase_entry()" << std::endl;

		for (auto it: _list_model_vectors->children())
		{
			Gtk::TreeModel::Row row = *it;

			if (row[_tree_model_vectors._column_id] == id)
			{
				_list_model_vectors->erase(it);
				break;
			}
		}
	}

	void MainControl::up()
	{
		db<MainControl>(TRC) << "MainControl::up()" << std::endl;

		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = (_shape_selected ? 1 : -1) * spin->get_value();

		auto T = model::transformation::translation(model::Vector(0, step));

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

	void MainControl::left()
	{
		db<MainControl>(TRC) << "MainControl::left()" << std::endl;

		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = (_shape_selected ? -1 : 1) * spin->get_value();

		auto T = model::transformation::translation(model::Vector(step, 0));

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

	void MainControl::right()
	{
		db<MainControl>(TRC) << "MainControl::right()" << std::endl;

		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = (_shape_selected ? 1 : -1) * spin->get_value();

		auto T = model::transformation::translation(model::Vector(step, 0));

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

	void MainControl::down()
	{
		db<MainControl>(TRC) << "MainControl::down()" << std::endl;

		Gtk::SpinButton *spin;
		_builder->get_widget("spin_step", spin);

		double step = (_shape_selected ? -1 : 1) * spin->get_value();

		auto T = model::transformation::translation(model::Vector(0, step));

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

	void MainControl::zoom_in()
	{
		db<MainControl>(TRC) << "MainControl::zoom_in()" << std::endl;

		Gtk::SpinButton *spin;
		_builder->get_widget("spin_percentual", spin);

		double times = spin->get_value();
		auto mass_center = _shapes_map[_shape_selected]->mass_center();
		
		auto T = model::transformation::scheduling(times, mass_center);

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

	void MainControl::zoom_out()
	{
		db<MainControl>(TRC) << "MainControl::zoom_out()" << std::endl;

		Gtk::SpinButton *spin;
		_builder->get_widget("spin_percentual", spin);

		double times = 1 / spin->get_value();
		auto mass_center = _shapes_map[_shape_selected]->mass_center();

		auto T = model::transformation::scheduling(times, mass_center);

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

	void MainControl::clockwise()
	{
		db<MainControl>(TRC) << "MainControl::clockwise()" << std::endl;

		Gtk::Entry *entry;
		Gtk::SpinButton *spin;
		Gtk::RadioButton *radio;
		model::Vector mass_center;

		/* Calculate the angle */
		_builder->get_widget("spin_degrees", spin);
		double angle = (_shape_selected ? -1 : 1) * spin->get_value() * (M_PI / 180);

		/* Calculate the center of mass */
		unsigned hash = 0;

		_builder->get_widget("radio_center_object", radio);
		hash += radio->get_active() ? ButtonID::CenterObject     : ButtonID::Null;

		_builder->get_widget("radio_center_world", radio);
		hash += radio->get_active() ? ButtonID::CenterWorld      : ButtonID::Null;

		_builder->get_widget("radio_center_specific", radio);
		hash += radio->get_active() ? ButtonID::CenterSpecific : ButtonID::Null;

		switch (hash)
		{
			case ButtonID::CenterObject:
				mass_center = _shapes_map[_shape_selected]->mass_center();
				break;

			case ButtonID::CenterWorld:
				mass_center = model::Vector(0, 0);
				break;

			case ButtonID::CenterSpecific: {
				double x{0}, y{0}, z{model::Vector::z};

				_builder->get_widget("entry_rotation_x", entry);
				x = atof(std::string(entry->get_text()).c_str());

				_builder->get_widget("entry_rotation_y", entry);
				y = atof(std::string(entry->get_text()).c_str());

				if (model::Vector::dimension == 4)
				{
					_builder->get_widget("entry_rotation_z", entry);
					z = atof(std::string(entry->get_text()).c_str());
				}
				
				mass_center = model::Vector(x, y, z);
			} break;

			/* Undefined */
			default:
				return;
		}

		/* Calculate the rotation matrix */
		auto T = model::transformation::rotation(angle, mass_center);

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

	void MainControl::counterclockwise()
	{
		db<MainControl>(TRC) << "MainControl::counterclockwise()" << std::endl;

		model::Vector mass_center;
		Gtk::Entry *entry;
		Gtk::SpinButton *spin;
		Gtk::RadioButton *radio;

		/* Calculate the angle */
		_builder->get_widget("spin_degrees", spin);
		double angle = (_shape_selected ? 1 : -1) * spin->get_value() * (M_PI / 180);

		/* Calculate the center of mass */
		unsigned hash = 0;

		_builder->get_widget("radio_center_object", radio);
		hash += radio->get_active() ? ButtonID::CenterObject     : ButtonID::Null;

		_builder->get_widget("radio_center_world", radio);
		hash += radio->get_active() ? ButtonID::CenterWorld      : ButtonID::Null;

		_builder->get_widget("radio_center_specific", radio);
		hash += radio->get_active() ? ButtonID::CenterSpecific : ButtonID::Null;

		switch (hash)
		{
			case ButtonID::CenterObject:
				mass_center = _shapes_map[_shape_selected]->mass_center();
				break;

			case ButtonID::CenterWorld:
				mass_center = model::Vector(0, 0);
				break;

			case ButtonID::CenterSpecific: {
				double x{0}, y{0}, z{model::Vector::z};

				_builder->get_widget("entry_rotation_x", entry);
				x = atof(std::string(entry->get_text()).c_str());

				_builder->get_widget("entry_rotation_y", entry);
				y = atof(std::string(entry->get_text()).c_str());

				if (model::Vector::dimension == 4)
				{
					_builder->get_widget("entry_rotation_z", entry);
					z = atof(std::string(entry->get_text()).c_str());
				}
				
				mass_center = model::Vector(x, y, z);
			} break;

			/* Undefined */
			default:
				return;
		}

		/* Calculate the rotation matrix */
		auto T = model::transformation::rotation(angle, mass_center);

		if (_shape_selected)
		{
			_shapes_map[_shape_selected]->transformation(T);
			build_objects({_shapes_map[_shape_selected]});
		}
		else
		{
			_window->transformation(T);
			build_objects(_shapes);
		}

		_viewport->update();
	}

} //! namespace control

#endif  // CONTROL_MAIN_CONTROL_HPP
