/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "PerspectiveCamera.hpp"

#include "minko/scene/Node.hpp"
#include "minko/scene/NodeSet.hpp"
#include "minko/controller/Surface.hpp"

using namespace minko::controller;
using namespace minko::math;

PerspectiveCamera::PerspectiveCamera(float fov,
                                     float aspectRatio,
                                     float zNear,
                                     float zFar) :
    _enabled(true),
	_view(math::Matrix4x4::create()),
	_projection(math::Matrix4x4::create()->perspective(fov, aspectRatio, zNear, zFar)),
	_viewProjection(math::Matrix4x4::create()->copyFrom(_projection)),
	_data(data::Provider::create()),
    _surfaces(scene::NodeSet::create(scene::NodeSet::Mode::AUTO))
{
    _surfaces
        ->root()
        ->descendants(true)
        ->hasController<Surface>();

	_data
		->set("transform/viewMatrix",			_view)
		->set("transform/projectionMatrix",		_projection)
		->set("transform/worldToScreenMatrix",	_viewProjection);
}

void
PerspectiveCamera::initialize()
{
	_targetAddedSlot = targetAdded()->connect(std::bind(
		&PerspectiveCamera::targetAddedHandler,
        shared_from_this(),
        std::placeholders::_1,
        std::placeholders::_2
	));
	_targetRemovedSlot = targetRemoved()->connect(std::bind(
		&PerspectiveCamera::targetAddedHandler,
        shared_from_this(),
        std::placeholders::_1,
        std::placeholders::_2
	));

    _surfaceAddedSlot = _surfaces->nodeAdded()->connect(std::bind(
        &PerspectiveCamera::surfaceAdded,
        shared_from_this(),
        std::placeholders::_1,
        std::placeholders::_2
    ));
    _surfaceRemovedSlot = _surfaces->nodeRemoved()->connect(std::bind(
        &PerspectiveCamera::surfaceRemoved,
        shared_from_this(),
        std::placeholders::_1,
        std::placeholders::_2
    ));
}

void
PerspectiveCamera::targetAddedHandler(AbstractController::Ptr ctrl,
                                      scene::Node::Ptr        target)
{
	if (targets().size() > 1)
		throw std::logic_error("PerspectiveCamera cannot have more than 1 target.");

	target->data()->addProvider(_data);
	target->data()->propertyChanged("transform/modelToWorldMatrix")->connect(std::bind(
		&PerspectiveCamera::localToWorldChangedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2
	));

    _surfaces->select(targets().begin(), targets().end())->update();
}

void
PerspectiveCamera::targetRemovedHandler(AbstractController::Ptr   ctrl,
                                        scene::Node::Ptr          target)
{
	target->data()->addProvider(_data);

    _surfaces->select(targets().begin(), targets().end())->update();
}

void
PerspectiveCamera::localToWorldChangedHandler(data::Container::Ptr	data,
											  const std::string&	propertyName)
{
	std::cout << "PerspectiveCamera::localToWorldChangedHandler()" << std::endl;

	_view->copyFrom(data->get<Matrix4x4::Ptr>("transform/modelToWorldMatrix"))->invert();
	_viewProjection->copyFrom(_view)->append(_projection);
}

void
PerspectiveCamera::surfaceAdded(scene::NodeSet::Ptr set, scene::Node::Ptr node)
{
    std::cout << "surface added: " << node->name() << std::endl;

    node->data()->addProvider(_data);
}

void
PerspectiveCamera::surfaceRemoved(scene::NodeSet::Ptr set, scene::Node::Ptr node)
{
    std::cout << "surface removed: " << node->name() << std::endl;

    node->data()->removeProvider(_data);
}