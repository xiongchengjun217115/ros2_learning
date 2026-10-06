from demo_python_pkg.person_node import PersonNode
class WriterNode(PersonNode):
    def __init__(self,book:str,name:str,age:int)->None: 
        print('WriterNode 的 __init__方法被调用了')
        self.book =book
        super().__init__('writer_node',name,age)
def main():
    import rclpy
    rclpy.init()
    node=WriterNode('xcj\'s autobiography','xcj',19)
    node.eat('鱼香ROS')
    rclpy.spin(node)
    rclpy.shutdown()        