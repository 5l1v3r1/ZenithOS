
class CUDPTreeQueue
{ // next, last for CQueue implementation.
	CUDPTreeQueue *next;
	CUDPTreeQueue *last;

	CUDPSocket* socket;
};

class CUDPTreeNode
{
	I64 port;

	CUDPTreeNode *left;
	CUDPTreeNode *right;

	CUDPTreeQueue* queue;
};

CUDPTreeNode *UDPTreeNodeInit()
{ // init new empty tree/node.
	CUDPTreeNode *tree_node = CAlloc(sizeof(CUDPTreeNode));

	return tree_node;
}

U0 UDPTreeNodeAdd(CUDPTreeNode *node, CUDPTreeNode *tree)
{ // using temp and last allows avoiding recursion and non-growing stack issues.
	CUDPTreeNode *temp_tree = tree;
	CUDPTreeNode *last_tree = temp_tree;

	while (temp_tree)
	{
		if (node->port < temp_tree->port)
		{ // if node smaller, go left
			last_tree = temp_tree;
			temp_tree = temp_tree->left;
		}
		else
		{ // if node equal or larger, go right
			last_tree = temp_tree;
			temp_tree = temp_tree->right;
		}	// at the end of this, this _should_ result in last_tree
	}		// being the resulting tree to store the node inside of. i guess recompute the direction and set.

	if (node->port < last_tree->port)// if node smaller, go left
		last_tree->left = node;
	else // if node equal or larger, go right
		last_tree->right = node;
}

CUDPTreeNode *UDPTreeNodeParamAdd(I64 node_port, CUDPTreeNode *tree)
{ // add a node using params, return pointer to the node
	CUDPTreeNode *result = UDPTreeNodeInit;
	result->port = node_port;

	UDPTreeNodeAdd(result, tree);

	return result;
}

CUDPTreeNode *UDPTreeNodeParamInit(I64 port)
{
	CUDPTreeNode *result = UDPTreeNodeInit;
	result->port = port;

	return result;
}

CUDPTreeNode *UDPTreeNodeFind(I64 port, CUDPTreeNode *tree)
{
	CUDPTreeNode *temp_tree = tree;

	while (temp_tree)
	{
		if (port < temp_tree->port) // if value smaller, go left
			temp_tree = temp_tree->left;
		else if (port > temp_tree->port) // if value larger, go right
			temp_tree = temp_tree->right;
		else // if value equal, match! i guess?
			break;
	}
	return temp_tree; // ! NULL if not found.
}

U0 UDPTreeNodeQueueInit(CUDPTreeNode *node)
{
	node->queue = CAlloc(sizeof(CUDPTreeQueue));
	QueueInit(node->queue);
}

U0 UDPTreeNodeQueueAdd(CUDPSocket* socket, CUDPTreeNode *node)
{
	CUDPTreeQueue *new_entry;

	if (!node->queue)
	{
		UDPTreeNodeQueueInit(node);
		node->queue->socket = socket;
	}
	else
	{
		new_entry = CAlloc(sizeof(CUDPTreeQueue));
		QueueInit(new_entry);
		new_entry->socket = socket;
		QueueInsert(new_entry, node->queue->last);
	}
}

// maybe not do find queue based on a socket ? am i brainfarting or 
// is there a more specific thing we should make a search for
// maybe what we need is a search by receive address IPV4/IPV6 ?
CUDPTreeQueue *UDPTreeNodeQueueFind(CSocket* socket, CUDPTreeNode *node)
{

	CUDPTreeQueue *temp_queue;

	if (node->queue)
	{

		if (node->queue->socket == socket)// im writing this but something feels weird about it
			return node->queue;

		temp_queue = node->queue->next;
		while (temp_queue != node->queue)
		{
			if (temp_queue->socket == socket)
				return temp_queue;
			temp_queue = temp_queue->next;
		}
	}

	return NULL;
}













































$SP,"<1>",BI=1$













































              �      �   3   �   N   �   e   �   |   �   �   b   �   �   �   =   �   c   �   �   �   �   �   �   �   �   �   �   �   �   �   �   s   �   �   �   �     �     �   $  �   �   @     V     ]   7  u   m  {   �  �   X  �   d  �   �  �   �  �   l  �   �  �   �   =   �   H   �   r   �   x   �   �   �   �   g   �   b   �   �   m   �   s   �   �   �   �   �   �   �   �     �   
  �   �   �   �   �   �   �   �   �   �   =   �   G     Z     l   ?  s   ^  �   e  �   ^  �   |  �     �   �  �   x  �   �   @   	   �   s      �   �   	   z   �      Y        �        �        �   �   	   �   �   	   �   �        �      �   c   	   ,  �      r  �      U  �      �  �   	   z  �      �   5   �   >   �   C   �   M   �   s   �   ~   g  �   o  �   v  �   z  �   �  �   �  �   y    }    k  �   p    �  �   �  �   �  �   �  �   �  �   �  �   X  �   a  �   F  �   R  �   +  �   3  �   �   d   �   o   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   �   |   �   �   �   \   
  b     �     �     �   	  �       �     �   �   �   �   �   n  �   t  �   �   ^   �   c      |   %  �   �   �   �   �   �   �   �   �    